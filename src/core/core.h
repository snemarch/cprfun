#pragma once
#ifndef cprfun__core_h
#define cprfun__core_h

#include <array>
#include <cassert>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace cprfun {

static const unsigned days_per_year = 366;										// to avoid dealing with leap years

void runpermutations(uint32_t start, uint32_t len, bool exhaustive, std::function<bool(const char*)> func);

class Hash {
public:
	static const size_t hashlength = 256/8;

	Hash();
	Hash(const void *data, size_t length);
	Hash(const Hash& other);

	void assign(const Hash& other);
	bool equals(const Hash& other) const;
	std::string toString() const;

	const void* getHash() const;

	static Hash fromHexString(const std::string& input);

private:
	std::array<std::uint8_t, hashlength> hash;

};

inline bool operator== (const Hash& lhs, const Hash& rhs)
{
	return lhs.equals(rhs);
}

inline bool operator!= (const Hash& lhs, const Hash& rhs)
{
	return !lhs.equals(rhs);
}


/******************************************************************************
* determines if a CPR string is conforms to DDMMYY-XXXX or DDMMYYXXXX
*
* returns true if cpr is valid
******************************************************************************/
bool isValidCpr(const std::string& cpr);

/******************************************************************************
* returns a Hash from a CPR string (DDMMYY-XXXX and DDMMYYXXXX both supported)
*
* Throws somethingsomething on error
******************************************************************************/
Hash hashFromCpr(const std::string& cpr);



class StopWatch
{
public:
	StopWatch();
	~StopWatch();

	void			reset();
	void			start();
	void			stop();
	uint64_t		getMilli() const;
	std::string		getFriendly() const;

private:
	class Impl;
	std::unique_ptr<Impl> impl;
};



/*******************************************************************************
 * Generates leading-0-filled base10 string representation of a fixed-width/size
 * unsigned integer input.
 * Guarantees against buffer overflows, but it's up to the caller to ensure that 
 * the number first within the give bounds, or results will be incorrect.
 ******************************************************************************/
template <unsigned int N> inline void base10fixWidthStr(char output[N], unsigned input)
{
	// assert( input+1 <= pow(10, N) ); // don't really want to depend on <math> and floating-point numbers.
	for(unsigned i=0; i<N; ++i)
	{
		output[N-i-1] = (input % 10) + '0';
		input /= 10;
	}
	assert (input == 0);
}


} // namespace cprfun

#endif // cprfun__core_h
