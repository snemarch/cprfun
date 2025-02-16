#pragma once
#ifndef cprfun__pycrypto_sha256_h
#define cprfun__pycrypto_sha256_h

//NOTE: this header is a gutted version from pycrypto, see comment section in pycrypto_SHA256.cpp

#define DIGEST_SIZE (256/8)
#define BLOCK_SIZE (512/8)
#define WORD_SIZE 4
#define SCHEDULE_SIZE 64

#ifndef DIGEST_SIZE
#error SHA2 Implementation must define DIGEST_SIZE before including this header
#else
#define DIGEST_SIZE_BITS (DIGEST_SIZE*8)
#endif

#ifndef BLOCK_SIZE
#error SHA2 Implementation must define BLOCK_SIZE before including this header
#else
#define BLOCK_SIZE_BITS (BLOCK_SIZE*8)
#endif

#ifndef WORD_SIZE
#error SHA2 Implementation must define WORD_SIZE before including this header
#else
#if ((WORD_SIZE != 4) && (WORD_SIZE != 8))
#error WORD_SIZE must be either 4 or 8
#else
#define WORD_SIZE_BITS (WORD_SIZE*8)
#endif
#endif

#ifndef SCHEDULE_SIZE
#error SHA2 Implementation must define SCHEDULE_SIZE before including this header
#endif

/* define some helper macros */
#define PADDING_SIZE (2 * WORD_SIZE)
#define LAST_BLOCK_SIZE (BLOCK_SIZE - PADDING_SIZE)

/* define generic SHA-2 family functions */
#define Ch(x,y,z)   ((x & y) ^ (~x & z))
#define Maj(x,y,z)  ((x & y) ^ (x & z) ^ (y & z))
#define ROTR(x, n)  (((x)>>((n)&(WORD_SIZE_BITS-1)))|((x)<<(WORD_SIZE_BITS-((n)&(WORD_SIZE_BITS-1)))))
#define SHR(x, n)   ((x)>>(n))

/* determine fixed size types */
#if defined(_MSC_VER)
    typedef unsigned char		U8;
typedef unsigned __int64	U64;
typedef unsigned int		U32;
#elif defined(__sun) || defined(__sun__)
#include <sys/inttypes.h>
typedef uint8_t				U8;
typedef uint32_t			U32;
typedef uint64_t			U64;
#else
#include <stdint.h>
typedef uint8_t				U8;
typedef uint32_t			U32;
typedef uint64_t			U64;
#endif

/* typedef a sha2_word_t type of appropriate size */
#if (WORD_SIZE_BITS == 64)
typedef U64 sha2_word_t;
#elif (WORD_SIZE_BITS == 32)
typedef U32 sha2_word_t;
#else
#error According to the FIPS Standard WORD_SIZE_BITS must be either 32 or 64
#endif

namespace cprfun::pycrypto {

/* define the hash_state structure */
struct alignas(32) hash_state {
    std::array<U8, BLOCK_SIZE> buf;
    std::array<sha2_word_t, 8> state;
    uint_fast32_t curlen;
    sha2_word_t length_upper, length_lower;
};

void sha_init(hash_state * hs);
void sha_process(hash_state * hs, const U8 *buf, int len);
void sha_done(hash_state * hs, unsigned char *hash);

}

#endif
