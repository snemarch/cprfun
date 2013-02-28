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
	HashStore(HashStore&& other);
	~HashStore();

	static HashStore createNew(const std::string& filename);
	static HashStore openExisting(const std::string& filename);

	void put(const Hash& hash, const std::string& cpr);
	bool tryGet(const Hash&hash, std::string *cpr);

private:
	HashStore();

	class Impl;
	std::unique_ptr<Impl> impl;
};


} // namespace cprfun


#endif // cprfun__hashstore_h
