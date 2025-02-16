#pragma once
#ifndef cprfun__sodium_sha256_h
#define cprfun__sodium_sha256_h

#include "SHA256.h"
#include <memory>

namespace cprfun::Sodium {

struct hash_state;

class SHA256Sodium final : public sha256 {
    std::unique_ptr<hash_state> state;

public:
    SHA256Sodium();
    ~SHA256Sodium();

    void reset();
    void update(const void *buffer, size_t length);
    void digest(digest_t& digest);
};

}

#endif
