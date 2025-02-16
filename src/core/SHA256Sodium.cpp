#include "stdafx.h"
#include "SHA256.h"

#include "SHA256Sodium.h""
#if FEAT_LIBSODIUM
#include "sodium.h"
#endif

namespace cprfun::Sodium {

struct hash_state {
};

SHA256Sodium::SHA256Sodium() {
}

SHA256Sodium::~SHA256Sodium() {
}

void SHA256Sodium::reset() {
}

void SHA256Sodium::update(const void *buffer, size_t length) {
}

void SHA256Sodium::digest(digest_t& digest) {
}

} // namespace cprfun
