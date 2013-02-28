#include "stdafx.h"
#include "hashstore.h"
#include "sqlite3.h"

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
		sqlite3_close(handle);
	}

private:
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
	//TODO: implement
	HashStore result;

	// CREATE TABLE hashmap (hash BLOB(32) PRIMARY KEY COLLATE BINARY, cpr INTEGER);
	// ref http://www.sqlite.org/lang_createtable.html
	// http://blog.quibb.org/2010/08/fast-bulk-inserts-into-sqlite/ for speed pragmas
	// examine if it's better to 'CREATE INDEX hashindex ON hashmap(hash)' after initial db construction.
	// examine if it's better to use TEXT instead of blob - obviously no-go if TEXT can't handle embedded NULs.

	return result;
}

HashStore HashStore::openExisting(const std::string& filename)
{
	//TODO: implement
	HashStore result;

	return result;
}

void HashStore::put(const Hash& hash, const std::string& cpr)
{
	//TODO: implement - insert via prepared statement
	//TODO: bind blobs - http://www.sqlite.org/c3ref/bind_blob.html
	//TODO: figure out if we should use transactions (are they necessary when using mass-create tuning pragmas? Can
	//		SQLite cope with a 366-million transaction?), and if so how it should be implemented (could be done as
	//		part of createNew, which seems a bit dirty - OTOH, RAII-style transaction class is overkill for this.)
	//		If there's anything to be gained, but 366million is overkill, an N-size cache could be implemented.
}

bool HashStore::tryGet(const Hash& hash, std::string *cpr)
{
	//TODO: implement
	return false;
}

} // namespace cprfun
