#pragma once
#ifndef cprfun__sha256_h
#define cprfun__sha256_h

#include <array>
#include <memory>

namespace cprfun
{

namespace pycrypto { struct hash_state; }

class sha256 {
public:
	static const size_t digest_bytelen = 256/8;
	typedef std::array<uint8_t, digest_bytelen> digest_t;

	sha256();
	~sha256();

	void reset();
	void update(const void *buffer, size_t length);
	void digest(digest_t& digest);

private:
	std::unique_ptr<pycrypto::hash_state> state;
};

}

#endif // cprfun__sha256_h
