#include "stdafx.h"
#include "SHA256.h"

#include <pycrypto_SHA256.h>

namespace cprfun
{

using namespace cprfun::pycrypto;

sha256::sha256() : state(std::make_unique<hash_state>())
{
	sha_init(state.get());
}

sha256::~sha256() = default;

void sha256::reset() {
	sha_init(state.get());
}

void sha256::update(const void *buf, size_t length)
{
	sha_process(state.get(), static_cast<const U8*>(buf), length);;
}

void sha256::digest(digest_t& digest)
{
	hash_state temp { *state };
	sha_done(&temp, &digest[0]);
}

}
