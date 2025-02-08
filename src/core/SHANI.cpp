#include "stdafx.h"
#include "SHA256.h"

# include <nmmintrin.h>
# include <immintrin.h>

#define M128_CAST(x) ((__m128i *)(void *)(x))
#define CONST_M128_CAST(x) ((const __m128i *)(const void *)(x))
#define W64LIT(x) x##ui64

namespace cprfun {

class shani: public sha256 {
public:
	// One day, I shall once again try and make this work :P

	/*
	void shanini(void* data, size_t length) {
		__m128i STATE0, STATE1;
		__m128i MSG, TMP, MASK;
		__m128i TMSG0, TMSG1, TMSG2, TMSG3;
		__m128i ABEF_SAVE, CDGH_SAVE;

		// Load initial values
		TMP = _mm_loadu_si128(M128_CAST(&state[0]));
		STATE1 = _mm_loadu_si128(M128_CAST(&state[4]));

		// IA-32 SHA is little endian, SHA::Transform is big endian,
		// and SHA::HashMultipleBlocks can be either. ByteOrder
		// allows us to avoid extra endian reversals. It saves 1.0 cpb.
		MASK = false ? //order == BIG_ENDIAN_ORDER ?  // Data arrangement
			_mm_set_epi8(12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3) :
			_mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);

		TMP = _mm_shuffle_epi32(TMP, 0xB1);          // CDAB
		STATE1 = _mm_shuffle_epi32(STATE1, 0x1B);    // EFGH
		STATE0 = _mm_alignr_epi8(TMP, STATE1, 8);    // ABEF
		STATE1 = _mm_blend_epi16(STATE1, TMP, 0xF0); // CDGH

		//We're only doing a single block, so no loopy loop
		//while (length >= SHA256::BLOCKSIZE)
		//{
			// Save current hash
			ABEF_SAVE = STATE0;
			CDGH_SAVE = STATE1;

			// Rounds 0-3
			MSG = _mm_loadu_si128(CONST_M128_CAST(data + 0));
			TMSG0 = _mm_shuffle_epi8(MSG, MASK);
			MSG = _mm_add_epi32(TMSG0, _mm_set_epi64x(W64LIT(0xE9B5DBA5B5C0FBCF), W64LIT(0x71374491428A2F98)));
			STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
			MSG = _mm_shuffle_epi32(MSG, 0x0E);
			STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);

			// Rounds 4-7
			TMSG1 = _mm_loadu_si128(CONST_M128_CAST(data + 4));
			TMSG1 = _mm_shuffle_epi8(TMSG1, MASK);
			MSG = _mm_add_epi32(TMSG1, _mm_set_epi64x(W64LIT(0xAB1C5ED5923F82A4), W64LIT(0x59F111F13956C25B)));
			STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
			MSG = _mm_shuffle_epi32(MSG, 0x0E);
			STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
			TMSG0 = _mm_sha256msg1_epu32(TMSG0, TMSG1);

			// Rounds 8-11
			TMSG2 = _mm_loadu_si128(CONST_M128_CAST(data + 8));
			TMSG2 = _mm_shuffle_epi8(TMSG2, MASK);
			MSG = _mm_add_epi32(TMSG2, _mm_set_epi64x(W64LIT(0x550C7DC3243185BE), W64LIT(0x12835B01D807AA98)));
			STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
			MSG = _mm_shuffle_epi32(MSG, 0x0E);
			STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
			TMSG1 = _mm_sha256msg1_epu32(TMSG1, TMSG2);

			// Rounds 12-15
			TMSG3 = _mm_loadu_si128(CONST_M128_CAST(data + 12));
			TMSG3 = _mm_shuffle_epi8(TMSG3, MASK);
			MSG = _mm_add_epi32(TMSG3, _mm_set_epi64x(W64LIT(0xC19BF1749BDC06A7), W64LIT(0x80DEB1FE72BE5D74)));
			STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
			TMP = _mm_alignr_epi8(TMSG3, TMSG2, 4);
			TMSG0 = _mm_add_epi32(TMSG0, TMP);
			TMSG0 = _mm_sha256msg2_epu32(TMSG0, TMSG3);
			MSG = _mm_shuffle_epi32(MSG, 0x0E);
			STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
			TMSG2 = _mm_sha256msg1_epu32(TMSG2, TMSG3);

			// Rounds 16-19
			MSG = _mm_add_epi32(TMSG0, _mm_set_epi64x(W64LIT(0x240CA1CC0FC19DC6), W64LIT(0xEFBE4786E49B69C1)));
			STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
			TMP = _mm_alignr_epi8(TMSG0, TMSG3, 4);
			TMSG1 = _mm_add_epi32(TMSG1, TMP);
			TMSG1 = _mm_sha256msg2_epu32(TMSG1, TMSG0);
			MSG = _mm_shuffle_epi32(MSG, 0x0E);
			STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
			TMSG3 = _mm_sha256msg1_epu32(TMSG3, TMSG0);

			// Rounds 20-23
			MSG = _mm_add_epi32(TMSG1, _mm_set_epi64x(W64LIT(0x76F988DA5CB0A9DC), W64LIT(0x4A7484AA2DE92C6F)));
			STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
			TMP = _mm_alignr_epi8(TMSG1, TMSG0, 4);
			TMSG2 = _mm_add_epi32(TMSG2, TMP);
			TMSG2 = _mm_sha256msg2_epu32(TMSG2, TMSG1);
			MSG = _mm_shuffle_epi32(MSG, 0x0E);
			STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
			TMSG0 = _mm_sha256msg1_epu32(TMSG0, TMSG1);

			// Rounds 24-27
			MSG = _mm_add_epi32(TMSG2, _mm_set_epi64x(W64LIT(0xBF597FC7B00327C8), W64LIT(0xA831C66D983E5152)));
			STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
			TMP = _mm_alignr_epi8(TMSG2, TMSG1, 4);
			TMSG3 = _mm_add_epi32(TMSG3, TMP);
			TMSG3 = _mm_sha256msg2_epu32(TMSG3, TMSG2);
			MSG = _mm_shuffle_epi32(MSG, 0x0E);
			STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
			TMSG1 = _mm_sha256msg1_epu32(TMSG1, TMSG2);

			// Rounds 28-31
			MSG = _mm_add_epi32(TMSG3, _mm_set_epi64x(W64LIT(0x1429296706CA6351), W64LIT(0xD5A79147C6E00BF3)));
			STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
			TMP = _mm_alignr_epi8(TMSG3, TMSG2, 4);
			TMSG0 = _mm_add_epi32(TMSG0, TMP);
			TMSG0 = _mm_sha256msg2_epu32(TMSG0, TMSG3);
			MSG = _mm_shuffle_epi32(MSG, 0x0E);
			STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
			TMSG2 = _mm_sha256msg1_epu32(TMSG2, TMSG3);

			// Rounds 32-35
			MSG = _mm_add_epi32(TMSG0, _mm_set_epi64x(W64LIT(0x53380D134D2C6DFC), W64LIT(0x2E1B213827B70A85)));
			STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
			TMP = _mm_alignr_epi8(TMSG0, TMSG3, 4);
			TMSG1 = _mm_add_epi32(TMSG1, TMP);
			TMSG1 = _mm_sha256msg2_epu32(TMSG1, TMSG0);
			MSG = _mm_shuffle_epi32(MSG, 0x0E);
			STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
			TMSG3 = _mm_sha256msg1_epu32(TMSG3, TMSG0);

			// Rounds 36-39
			MSG = _mm_add_epi32(TMSG1, _mm_set_epi64x(W64LIT(0x92722C8581C2C92E), W64LIT(0x766A0ABB650A7354)));
			STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
			TMP = _mm_alignr_epi8(TMSG1, TMSG0, 4);
			TMSG2 = _mm_add_epi32(TMSG2, TMP);
			TMSG2 = _mm_sha256msg2_epu32(TMSG2, TMSG1);
			MSG = _mm_shuffle_epi32(MSG, 0x0E);
			STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
			TMSG0 = _mm_sha256msg1_epu32(TMSG0, TMSG1);

			// Rounds 40-43
			MSG = _mm_add_epi32(TMSG2, _mm_set_epi64x(W64LIT(0xC76C51A3C24B8B70), W64LIT(0xA81A664BA2BFE8A1)));
			STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
			TMP = _mm_alignr_epi8(TMSG2, TMSG1, 4);
			TMSG3 = _mm_add_epi32(TMSG3, TMP);
			TMSG3 = _mm_sha256msg2_epu32(TMSG3, TMSG2);
			MSG = _mm_shuffle_epi32(MSG, 0x0E);
			STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
			TMSG1 = _mm_sha256msg1_epu32(TMSG1, TMSG2);

			// Rounds 44-47
			MSG = _mm_add_epi32(TMSG3, _mm_set_epi64x(W64LIT(0x106AA070F40E3585), W64LIT(0xD6990624D192E819)));
			STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
			TMP = _mm_alignr_epi8(TMSG3, TMSG2, 4);
			TMSG0 = _mm_add_epi32(TMSG0, TMP);
			TMSG0 = _mm_sha256msg2_epu32(TMSG0, TMSG3);
			MSG = _mm_shuffle_epi32(MSG, 0x0E);
			STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
			TMSG2 = _mm_sha256msg1_epu32(TMSG2, TMSG3);

			// Rounds 48-51
			MSG = _mm_add_epi32(TMSG0, _mm_set_epi64x(W64LIT(0x34B0BCB52748774C), W64LIT(0x1E376C0819A4C116)));
			STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
			TMP = _mm_alignr_epi8(TMSG0, TMSG3, 4);
			TMSG1 = _mm_add_epi32(TMSG1, TMP);
			TMSG1 = _mm_sha256msg2_epu32(TMSG1, TMSG0);
			MSG = _mm_shuffle_epi32(MSG, 0x0E);
			STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
			TMSG3 = _mm_sha256msg1_epu32(TMSG3, TMSG0);

			// Rounds 52-55
			MSG = _mm_add_epi32(TMSG1, _mm_set_epi64x(W64LIT(0x682E6FF35B9CCA4F), W64LIT(0x4ED8AA4A391C0CB3)));
			STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
			TMP = _mm_alignr_epi8(TMSG1, TMSG0, 4);
			TMSG2 = _mm_add_epi32(TMSG2, TMP);
			TMSG2 = _mm_sha256msg2_epu32(TMSG2, TMSG1);
			MSG = _mm_shuffle_epi32(MSG, 0x0E);
			STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);

			// Rounds 56-59
			MSG = _mm_add_epi32(TMSG2, _mm_set_epi64x(W64LIT(0x8CC7020884C87814), W64LIT(0x78A5636F748F82EE)));
			STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
			TMP = _mm_alignr_epi8(TMSG2, TMSG1, 4);
			TMSG3 = _mm_add_epi32(TMSG3, TMP);
			TMSG3 = _mm_sha256msg2_epu32(TMSG3, TMSG2);
			MSG = _mm_shuffle_epi32(MSG, 0x0E);
			STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);

			// Rounds 60-63
			MSG = _mm_add_epi32(TMSG3, _mm_set_epi64x(W64LIT(0xC67178F2BEF9A3F7), W64LIT(0xA4506CEB90BEFFFA)));
			STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
			MSG = _mm_shuffle_epi32(MSG, 0x0E);
			STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);

			// Add values back to state
			STATE0 = _mm_add_epi32(STATE0, ABEF_SAVE);
			STATE1 = _mm_add_epi32(STATE1, CDGH_SAVE);

		//	data += SHA256::BLOCKSIZE / sizeof(word32);
		//	length -= SHA256::BLOCKSIZE;
		//}

		TMP = _mm_shuffle_epi32(STATE0, 0x1B);       // FEBA
		STATE1 = _mm_shuffle_epi32(STATE1, 0xB1);    // DCHG
		STATE0 = _mm_blend_epi16(TMP, STATE1, 0xF0); // DCBA
		STATE1 = _mm_alignr_epi8(STATE1, TMP, 8);    // ABEF

		// Save state
		_mm_storeu_si128(M128_CAST(&state[0]), STATE0);
		_mm_storeu_si128(M128_CAST(&state[4]), STATE1);

	}
	*/
	};
}
