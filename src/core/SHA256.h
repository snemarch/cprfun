#pragma once
#ifndef cprfun__sha256_h
#define cprfun__sha256_h

#include <array>

namespace cprfun
{

class sha256 {
protected:
	sha256() = default;

public:
	static const size_t digest_bytelen = 256/8;
	typedef std::array<uint8_t, digest_bytelen> digest_t;

	virtual ~sha256() = default;

	virtual void reset() = 0;
	virtual void update(const void *buffer, size_t length) = 0;
	virtual void digest(digest_t& digest) = 0;
};

}

#endif // cprfun__sha256_h
