#include "stdafx.h"
#include "hashstore.h"
#include "sqlite3.h"
#include "WorkQueue.h"
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

namespace cprfun
{
using hashcache_t = std::pair<Hash, unsigned long>;

class HashStore::Impl
{
public:
	Impl() : handle(nullptr, &sqlite3_close), insert_stmt(nullptr, &sqlite3_finalize), workQueue(10'000)
	{
		workerThread = std::thread(&Impl::background_flush, this);
	}

	~Impl() {
		workQueue.shutdown();
		workerThread.join();
	}

	void open(const std::string& db, bool create)
	{
		// Open the database in NOMUTEX mode: multi-threaded where connections, statements etc aren't used by multiple
		// threads at the same time, so expensive/coarse-grained mutexes aren't used.
		sqlite3* db_handle;
		const int flags = SQLITE_OPEN_READWRITE | (create ? SQLITE_OPEN_CREATE : 0) | SQLITE_OPEN_NOMUTEX;
		if(SQLITE_OK != sqlite3_open_v2(db.c_str(), &db_handle, flags, nullptr))
		{
			std::string msg("can't open database [" + db + "] - error: " + sqlite3_errmsg(db_handle));
			sqlite3_close(db_handle);
			handle = nullptr;
			throw std::runtime_error(msg);
		}

		handle.reset(db_handle);
	}

	void exec(const std::string& statement)
	{
		char *errmsg = nullptr;

		if(SQLITE_OK != sqlite3_exec(handle.get(), statement.c_str(), nullptr, nullptr, &errmsg))
		{
			std::string msg(std::string("Database error: ") + errmsg);
			sqlite3_free(errmsg);
			throw std::runtime_error( msg );
		}
	}

	void putHash(const Hash& hash, const std::string& cpr)
	{
		workQueue.produce(hash, stoul(cpr));
	}

	bool tryGet(const Hash& hash, std::string *cpr)
	{
		//TODO: refactor common parts from get and put. Probably not a bad idea introducing a Statement class after all?
		static const std::string stmtText("SELECT cpr FROM hashmap WHERE hash = ?1;");

		sqlite3_stmt	*stmt;

		if(SQLITE_OK != sqlite3_prepare_v2(handle.get(), stmtText.c_str(), stmtText.size(), &stmt, nullptr))
		{
			std::string msg(std::string("error creating prepared statement: ") + sqlite3_errmsg(handle.get()));
			throw std::runtime_error(msg);
		}

		// ref - http://www.sqlite.org/c3ref/bind_blob.html
		sqlite3_bind_blob(stmt, 1, hash.getHash(), hash.hashlength, SQLITE_STATIC);

		while( SQLITE_ROW == sqlite3_step(stmt) )
		{
			unsigned numericCpr = static_cast<unsigned>( sqlite3_column_int(stmt, 0) );
			char rawCharCpr[11];
			rawCharCpr[10] = 0;
			base10fixWidthStr<10>(rawCharCpr, numericCpr);
			*cpr = rawCharCpr;
			std::cout << ".";
		}

		sqlite3_finalize(stmt);
		return true;
	}

	void buildIndex()
	{
		flushCache(true);
		exec("CREATE INDEX hashindex ON hashmap(hash);");
	}


private:
	std::unique_ptr<sqlite3, decltype(&sqlite3_close)> handle;
	std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> insert_stmt;
	cprfun::WorkQueue<hashcache_t> workQueue;
	std::thread workerThread;

	void flushCache(bool shutdown = false)
	{
		workQueue.flush(shutdown);
	}

	void initializeInsertStatement() {
		// Rearchitecture so this can be done *once* at a sensible place, without if guards.
		static const std::string stmtText("INSERT INTO hashmap VALUES(?1, ?2);");
		if (!insert_stmt)
		{
			sqlite3_stmt *stmt;
			if (SQLITE_OK != sqlite3_prepare_v2(handle.get(), stmtText.c_str(), stmtText.size(), &stmt, nullptr))
			{
				std::string msg(std::string("error creating prepared statement: ") + sqlite3_errmsg(handle.get()));
				throw std::runtime_error(msg);
			}
			insert_stmt.reset(stmt);
		}
	}

	void background_flush() {

		while (true) {
			auto *buffer = workQueue.consume();
			if (!buffer) {
				break;
			}

			initializeInsertStatement();

			exec("BEGIN TRANSACTION;");

			auto& work = *buffer;
			for (auto&[hash, cpr] : work)
			{
				// The item hash has lifetime until the cache is cleared, so we can use SQLITE_STATIC to avoid
				// the memory copying SQLITE_TRANSIENT does.
				sqlite3_bind_blob(insert_stmt.get(), 1, hash.getHash(), hash.hashlength, SQLITE_STATIC);
				sqlite3_bind_int(insert_stmt.get(), 2, cpr);

				if ( SQLITE_DONE != sqlite3_step(insert_stmt.get()) )
				{
					std::string msg(std::string("error executing prepared statement: ") + sqlite3_errmsg(handle.get()));
					sqlite3_finalize(insert_stmt.get());
					throw std::runtime_error(msg);
				}

				sqlite3_reset(insert_stmt.get());
			}

			exec("COMMIT TRANSACTION;");

			sqlite3_clear_bindings(insert_stmt.get());

			workQueue.done();
		}
	}
};

HashStore::HashStore(HashStore&& other) : impl( std::move(other.impl) )
{
}

HashStore::HashStore() : impl( std::make_unique<Impl>() )
{
}

HashStore::~HashStore() = default;


HashStore HashStore::createNew(const std::string& filename)
{
	HashStore result;

	result.impl->open(filename, true);
	result.impl->exec("CREATE TABLE hashmap (hash BLOB(32) COLLATE BINARY, cpr INTEGER);");
	//result.impl->exec("CREATE TABLE hashmap (hash BLOB(32) PRIMARY KEY COLLATE BINARY, cpr INTEGER) WITHOUT ROWID;"); // ekstrem lang buildtime, men mindre fil!
	// ref http://www.sqlite.org/lang_createtable.html
	// ref http://www.sqlite.org/pragma.html

	// Option: no journal, fast but a crash will likely corrupt the database. YOLO.
	result.impl->exec("PRAGMA journal_mode = OFF;");
	result.impl->exec("PRAGMA synchronous = OFF;");

	// Option: memory journal. Really old comment says this caused OOM crash, but with updated SQLite version it worked
	// and used less memory than the no-journal mode. It's slower and doesn't add much safety compared to disabling
	// journalling, so probably not of much use.
	//result.impl->exec("PRAGMA journal_mode = MEMORY;");

	// Option: WAL journal mode. Should be faster than default journalling. The journal_size_limit should probably be
	// tweaked, it was copied from some article that seemed to be focused on Android dev, rather than desktop systems.
	//result.impl->exec("PRAGMA journal_mode = WAL;");
	//result.impl->exec("PRAGMA synchronous = NORMAL;");
	//result.impl->exec("PRAGMA journal_size_limit = 6144000;");

	// More than 16 threads doesn't seem to provide much benefit on a 16-core hyperthreaded system.
	result.impl->exec("PRAGMA threads = 16;");
	result.impl->exec("PRAGMA page_size = 32768;");
	result.impl->exec("PRAGMA locking_mode=EXCLUSIVE"); // avoid constant acquire/release of file lock
	result.impl->exec("PRAGMA cache_size = -1024;");	// 1gig cache - not even 100meg used during insert or index creation, this is more than fine.
	//result.impl->exec("PRAGMA temp_store=MEMORY;");	// requires ~22.5gig process memory, seems to disable threading during index creation.

	return result;
}

HashStore HashStore::openExisting(const std::string& filename)
{
	HashStore result;

	result.impl->open(filename, false);

	return result;
}

void HashStore::put(const Hash& hash, const std::string& cpr)
{
	impl->putHash(hash, cpr);
}

bool HashStore::tryGet(const Hash& hash, std::string *cpr)
{
	return impl->tryGet(hash, cpr);
}

void HashStore::buildIndex()
{
	impl->buildIndex();
}


} // namespace cprfun
