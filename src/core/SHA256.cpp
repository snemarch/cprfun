#include "stdafx.h"
#include "SHA256.h"
#include <string.h> // memcpy
/*
 * An implementation of the SHA-256 hash function.
 *
 * The Federal Information Processing Standards (FIPS) Specification 
 * can be found here (FIPS 180-3):
 *   http://csrc.nist.gov/publications/PubsFIPS.html
 * 
 * Written in 2010 by Lorenz Quack <don@amberfisharts.com>
 *
 * ===================================================================
 * The contents of this file are dedicated to the public domain.  To
 * the extent that dedication to the public domain is not available,
 * everyone is granted a worldwide, perpetual, royalty-free,
 * non-exclusive license to exercise all rights associated with the
 * contents of this file for any purpose whatsoever.
 * No rights are reserved.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * ===================================================================
 *
 */
/*
 * CprFun Note: this file is from PyCrypto - http://www.pycrypto.org . Their
 * GitHub repository, https://github.com/dlitz/pycrypto, was the first google
 * hit for "public domain sha256 C". The various header & source files were
 * copy-pasta'ed into this monolithic block - easier and smaller footprint than
 * than finding a proper crypto library, compiling it, adding dependencies etc.
 *
 * Did I mention I'm lazy? :-)
 */

namespace cprfun
{
#define MODULE_NAME _SHA256
#define DIGEST_SIZE (256/8)
#define BLOCK_SIZE (512/8)
#define WORD_SIZE 4
#define SCHEDULE_SIZE 64
 
/*
 * An generic header for the SHA-2 hash family.
 *
 * Written in 2010 by Lorenz Quack <don@amberfisharts.com>
 * 
 * ===================================================================
 * The contents of this file are dedicated to the public domain.  To
 * the extent that dedication to the public domain is not available,
 * everyone is granted a worldwide, perpetual, royalty-free,
 * non-exclusive license to exercise all rights associated with the
 * contents of this file for any purpose whatsoever.
 * No rights are reserved.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * ===================================================================
 *
 */

#ifndef __HASH_SHA2_H
#define __HASH_SHA2_H

/* check if implementation set the correct macros */
#ifndef MODULE_NAME
#error SHA2 Implementation must define MODULE_NAME before including this header
#endif

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

/* define the hash_state structure */
struct hash_state {
	sha2_word_t state[8];
	int curlen;
	sha2_word_t length_upper, length_lower;
	unsigned char buf[BLOCK_SIZE];
};

#endif /* __HASH_SHA2_H */

/* Initial Values H */
static const sha2_word_t H[8] = {
	0x6a09e667,
	0xbb67ae85,
	0x3c6ef372,
	0xa54ff53a,
	0x510e527f,
	0x9b05688c,
	0x1f83d9ab,
	0x5be0cd19
};

/* the Constants K */
static const sha2_word_t K[SCHEDULE_SIZE] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b,
	0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01,
	0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7,
	0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
	0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152,
	0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
	0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc,
	0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819,
	0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116, 0x1e376c08,
	0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f,
	0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
	0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

/* SHA-256 specific functions */
#define Sigma0(x)    (ROTR(x,  2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define Sigma1(x)    (ROTR(x,  6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define Gamma0(x)    (ROTR(x,  7) ^ ROTR(x, 18) ^ SHR(x,  3))
#define Gamma1(x)    (ROTR(x, 17) ^ ROTR(x, 19) ^ SHR(x, 10))

/*
 * An generic implementation of the SHA-2 hash family, this is endian neutral
 * so should work just about anywhere.
 *
 * This code works much like the MD5 code provided by RSA.  You sha_init()
 * a "sha_state" then sha_process() the bytes you want and sha_done() to get
 * the output.
 *
 * Originally written by Tom St Denis -- http://tomstdenis.home.dhs.org
 * Adapted for PyCrypto by Jeethu Rao, Taylor Boon, and others.
 * Turned into a generic template by Lorenz Quack <don@amberfisharts.com>
 * 
 * ===================================================================
 * The contents of this file are dedicated to the public domain.  To
 * the extent that dedication to the public domain is not available,
 * everyone is granted a worldwide, perpetual, royalty-free,
 * non-exclusive license to exercise all rights associated with the
 * contents of this file for any purpose whatsoever.
 * No rights are reserved.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * ===================================================================
 *
 */


/* compress one block  */
static void sha_compress(hash_state * hs)
{
	sha2_word_t S[8], W[SCHEDULE_SIZE], T1, T2;
	int i;

	/* copy state into S */
	for (i = 0; i < 8; i++)
		S[i] = hs->state[i];

	/* copy the state into W[0..15] */
	for (i = 0; i < 16; i++){
		W[i] = (
			(((sha2_word_t) hs->buf[(WORD_SIZE*i)+0]) << (WORD_SIZE_BITS- 8)) |
			(((sha2_word_t) hs->buf[(WORD_SIZE*i)+1]) << (WORD_SIZE_BITS-16)) |
			(((sha2_word_t) hs->buf[(WORD_SIZE*i)+2]) << (WORD_SIZE_BITS-24)) |
			(((sha2_word_t) hs->buf[(WORD_SIZE*i)+3]) << (WORD_SIZE_BITS-32))
#if (WORD_SIZE_BITS == 64)
			|
			(((sha2_word_t) hs->buf[(WORD_SIZE*i)+4]) << (WORD_SIZE_BITS-40)) |
			(((sha2_word_t) hs->buf[(WORD_SIZE*i)+5]) << (WORD_SIZE_BITS-48)) |
			(((sha2_word_t) hs->buf[(WORD_SIZE*i)+6]) << (WORD_SIZE_BITS-56)) |
			(((sha2_word_t) hs->buf[(WORD_SIZE*i)+7]))
#endif
			);
	}    

	/* fill W[16..SCHEDULE_SIZE] */
	for (i = 16; i < SCHEDULE_SIZE; i++)
		W[i] = Gamma1(W[i - 2]) + W[i - 7] + Gamma0(W[i - 15]) + W[i - 16];

	/* Compress */
	for (i = 0; i < SCHEDULE_SIZE; i++) {
		T1 = S[7] + Sigma1(S[4]) + Ch(S[4], S[5], S[6]) + K[i] + W[i];
		T2 = Sigma0(S[0]) + Maj(S[0], S[1], S[2]);
		S[7] = S[6];
		S[6] = S[5];
		S[5] = S[4];
		S[4] = S[3] + T1;
		S[3] = S[2];
		S[2] = S[1];
		S[1] = S[0];
		S[0] = T1 + T2;
	}

	/* feedback */
	for (i = 0; i < 8; i++)
		hs->state[i] += S[i];
}

/* adds *inc* to the length of the hash_state *hs*
 * return 1 on success
 * return 0 if the length overflows
 */
static int add_length(hash_state *hs, sha2_word_t inc) {
	sha2_word_t overflow_detector;
	overflow_detector = hs->length_lower;
	hs->length_lower += inc;
	if (overflow_detector > hs->length_lower) {
		overflow_detector = hs->length_upper;
		hs->length_upper++;
		if (hs->length_upper > hs->length_upper)
			return 0;
	}
	return 1;
}

/* init the SHA state */
static void sha_init(hash_state * hs)
{
	int i;
	hs->curlen = hs->length_upper = hs->length_lower = 0;
	for (i = 0; i < 8; ++i)
		hs->state[i] = H[i];
}

static void sha_process(hash_state * hs, unsigned char *buf, int len)
{
	while (len--) {
		/* copy byte */
		hs->buf[hs->curlen++] = *buf++;

		/* is a block full? */
		if (hs->curlen == BLOCK_SIZE) {
			sha_compress(hs);
			add_length(hs, BLOCK_SIZE_BITS);
			hs->curlen = 0;
		}
	}
}

static void sha_done(hash_state * hs, unsigned char *hash)
{
	int i;

	/* increase the length of the message */
	add_length(hs, hs->curlen * 8);

	/* append the '1' bit */
	hs->buf[hs->curlen++] = 0x80;

	/* if the length is currently above LAST_BLOCK_SIZE bytes we append
	 * zeros then compress.  Then we can fall back to padding zeros and length
	 * encoding like normal.
	 */
	if (hs->curlen > LAST_BLOCK_SIZE) {
		for (; hs->curlen < BLOCK_SIZE;)
			hs->buf[hs->curlen++] = 0;
		sha_compress(hs);
		hs->curlen = 0;
	}

	/* pad upto LAST_BLOCK_SIZE bytes of zeroes */
	for (; hs->curlen < LAST_BLOCK_SIZE;)
		hs->buf[hs->curlen++] = 0;

	/* append length */
	for (i = 0; i < WORD_SIZE; i++)
		hs->buf[i + LAST_BLOCK_SIZE] = 
			(hs->length_upper >> ((WORD_SIZE - 1 - i) * 8)) & 0xFF;
	for (i = 0; i < WORD_SIZE; i++)
		hs->buf[i + LAST_BLOCK_SIZE + WORD_SIZE] = 
			(hs->length_lower >> ((WORD_SIZE - 1 - i) * 8)) & 0xFF;
	sha_compress(hs);

	/* copy output */
	for (i = 0; i < DIGEST_SIZE; i++)
		hash[i] = (hs->state[i / WORD_SIZE] >> 
				   ((WORD_SIZE - 1 - (i % WORD_SIZE)) * 8)) & 0xFF;
}

// Done
static void hash_init (hash_state *ptr)
{
	sha_init(ptr);
}

// Done
static void
hash_update (hash_state *self, const U8 *buf, int len)
{
	sha_process(self,(unsigned char *)buf, len);
}

// Done
static void
hash_copy(hash_state *src, hash_state *dest)
{
	memcpy(dest,src,sizeof(hash_state));
}

//cprfun: public interface methods - might seem silly to do it this way rather
//than replace the hash_* above, but this way we make as few changes to the
//original code as possible.
sha256::sha256() : state(std::make_unique<hash_state>())
{
	hash_init(state.get());
}

sha256::~sha256() = default;

void sha256::reset() {
	hash_init(state.get());
}

void sha256::update(const void *buf, size_t length)
{
	hash_update(state.get(), const_cast<U8*>(static_cast<const U8*>(buf)), length);
}

void sha256::digest(digest_t& digest)
{
	hash_state temp;

	hash_copy(state.get(), &temp);
	sha_done(&temp, &digest[0]);
}

}
