#pragma once
#ifndef cprfun__pycrypto_sha256_h
#define cprfun__pycrypto_sha256_h

#include "SHA256.h"
#include <memory>

namespace cprfun::PyCrypto {

struct hash_state;

class SHA256PyCrypto final : public sha256 {
	std::unique_ptr<hash_state> state;

public:
	SHA256PyCrypto();
	~SHA256PyCrypto();

	void reset();
	void update(const void *buffer, size_t length);
	void digest(digest_t& digest);
};

}

#endif
