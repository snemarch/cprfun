#include "stdafx.h"
#include "SHA256.h"

#include "SHA256Sodium.h"
#include "sodium.h"

namespace cprfun::Sodium {

struct hash_state: crypto_hash_sha256_state { };

SHA256Sodium::SHA256Sodium() : state(std::make_unique<hash_state>()) {
}

SHA256Sodium::~SHA256Sodium() = default;

void SHA256Sodium::reset() {
    crypto_hash_sha256_init(state.get());
}

void SHA256Sodium::update(const void *buffer, size_t length) {
    crypto_hash_sha256_update(state.get(), static_cast<const unsigned char *>(buffer), length);
}

void SHA256Sodium::digest(digest_t& digest) {
    crypto_hash_sha256_final(state.get(), digest.data());
}

} // namespace cprfun::Sodium
