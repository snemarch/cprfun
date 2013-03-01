#include "stdafx.h"
#include "hashstore.h"
#include "sqlite3.h"
#include <stdexcept>
#include <vector>
#include <iostream> //TODO: remove, temporary debug

namespace cprfun
{

class HashStore::Impl
{
public:
	Impl() : handle(nullptr)
	{
	}

	~Impl()
	{
		if(handle)
		{
			sqlite3_close(handle);
		}
	}

	void open(const std::string& db)
	{
		int rc = sqlite3_open(db.c_str(), &handle);
		if(rc)
		{
			std::string msg("can't open datase [" + db + "] - error: " + sqlite3_errmsg(handle));
			sqlite3_close(handle);
			handle = nullptr;
			throw std::runtime_error(msg);
		}
	}

	void exec(const std::string& statement)
	{
		char *errmsg = nullptr;

		if(SQLITE_OK != sqlite3_exec(handle, statement.c_str(), nullptr, nullptr, &errmsg))
		{
			std::string msg(std::string("Database error: ") + errmsg);
			sqlite3_free(errmsg);
			throw std::runtime_error( msg );
		}
	}

	void putHash(const Hash& hash, const std::string& cpr)
	{
		cache.push_back( std::make_pair(hash, stoul(cpr)) );
		if(cache.size() == 10000) {
			flushCache();
		}
	}

	bool tryGet(const Hash& hash, std::string *cpr)
	{
		//TODO: refactor common parts from get and put. Probably not a bad idea introducing a Statement class after all?
		static const std::string stmtText("SELECT cpr FROM hashmap WHERE hash = ?1;");

		sqlite3_stmt	*stmt;

		if(SQLITE_OK != sqlite3_prepare_v2(handle, stmtText.c_str(), stmtText.size(), &stmt, nullptr))
		{
			std::string msg(std::string("error creating prepared statement: ") + sqlite3_errmsg(handle));
			throw std::runtime_error(msg);
		}

		// ref - http://www.sqlite.org/c3ref/bind_blob.html
		sqlite3_bind_blob(stmt, 1, hash.getHash(), hash.hashlength, SQLITE_TRANSIENT);

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
		//debug with sqlite3 tool: select lower(quote(hash)), cpr from hashmap limit 10;
	}

	void buildIndex()
	{
		flushCache();
		exec("CREATE INDEX hashindex ON hashmap(hash);");
	}


private:
	void flushCache()
	{
		static const std::string stmtText("INSERT INTO hashmap VALUES (?1, ?2);");
		sqlite3_stmt	*stmt;

		// std::cout << "flushing " << cache.size() << " items" << std::endl;

		if(SQLITE_OK != sqlite3_prepare_v2(handle, stmtText.c_str(), stmtText.size(), &stmt, nullptr))
		{
			std::string msg(std::string("error creating prepared statement: ") + sqlite3_errmsg(handle));
			throw std::runtime_error(msg);
		}

		exec("BEGIN TRANSACTION;");

		for(auto& item : cache)
		{
			sqlite3_bind_blob(stmt, 1, item.first.getHash(), item.first.hashlength, SQLITE_TRANSIENT);
			sqlite3_bind_int(stmt, 2, item.second);

			if ( SQLITE_DONE != sqlite3_step(stmt) )
			{
				std::string msg(std::string("error executing prepared statement: ") + sqlite3_errmsg(handle));
				sqlite3_finalize(stmt);
				throw std::runtime_error(msg);
			}

			sqlite3_reset(stmt);
		}

		exec("COMMIT TRANSACTION;");
		sqlite3_finalize(stmt);
		cache.clear();
		//Right, cache size of 1000 items and commit: a factor of ~4:30 vs ~1:00

		//TODO: figure out if we should use transactions (are they necessary when using mass-create tuning pragmas? Can
		//		SQLite cope with a 366-million transaction?), and if so how it should be implemented (could be done as
		//		part of createNew, which seems a bit dirty - OTOH, RAII-style transaction class is overkill for this.)
		//		If there's anything to be gained, but 366million is overkill, an N-size cache could be implemented.
	}

	std::vector< std::pair<Hash, unsigned long>> cache;

	sqlite3		*handle;
};

HashStore::HashStore(HashStore&& other) : impl( std::move(other.impl) )
{
}

HashStore::HashStore() : impl(new Impl)
{
	//TODO: figure out the best time/place to create prepared statement for insert
}

HashStore::~HashStore() { }


HashStore HashStore::createNew(const std::string& filename)
{
	HashStore result;

	result.impl->open(filename);
	result.impl->exec("CREATE TABLE hashmap (hash BLOB(32) COLLATE BINARY, cpr INTEGER);");
	// ref http://www.sqlite.org/lang_createtable.html

//	result.impl->exec("PRAGMA journal_mode=MEMORY;");
	result.impl->exec("PRAGMA journal_mode=OFF;");
	result.impl->exec("PRAGMA synchronous=OFF;");
	result.impl->exec("PRAGMA temp_store=MEMORY;");
	result.impl->exec("PRAGMA locking_mode=EXCLUSIVE"); // avoid constant acquire/release of file lock
	result.impl->exec("PRAGMA page_size=4096;");
	result.impl->exec("PRAGMA cache_size=65536;");	// 256meg cache
	
	// http://blog.quibb.org/2010/08/fast-bulk-inserts-into-sqlite/ for speed pragmas
	// http://www.sqlite.org/pragma.html

	// 10k entries: 01:22:19 - 01:58:58, large period with extremely scattered disk I/O (and thus abysmal throuhgput) during middle of insert period. File ended up at 3.508.666.368 bytes, 156 fragments (lots less than expected!)
	
	// examine if it's better to 'CREATE INDEX hashindex ON hashmap(hash)' after initial db construction.
	// examine if it's better to use TEXT instead of blob - obviously no-go if TEXT can't handle embedded NULs.

	return result;
}

HashStore HashStore::openExisting(const std::string& filename)
{
	HashStore result;

	result.impl->open(filename);

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
