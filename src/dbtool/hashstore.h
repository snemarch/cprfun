#pragma once
#ifndef cprfun__hashstore_h
#define cprfun__hashstore_h

#include <memory>
#include <string>

#include <core/core.h>

namespace cprfun
{

class HashStore
{
public:
	HashStore(const HashStore& other) = delete;
	HashStore(HashStore&& other);
	~HashStore();
	HashStore& operator=(const HashStore& other) = delete;
	HashStore& operator=(HashStore&& other) = delete;

	static HashStore createNew(const std::string& filename);
	static HashStore openExisting(const std::string& filename);

	void put(const Hash& hash, const std::string& cpr);
	bool tryGet(const Hash& hash, std::string *cpr);
	void buildIndex();

private:
	HashStore();

	class Impl;
	std::unique_ptr<Impl> impl;
};


} // namespace cprfun


#endif // cprfun__hashstore_h
