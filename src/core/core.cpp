#include "stdafx.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <cctype>
#include <chrono>
#include <functional>

#include "core.h"
#include "SHA256.h"


namespace cprfun {

Hash::Hash()
{
	hash.fill(0);
}

Hash::Hash(const void *data, size_t length)
{
	sha256 sha;

	sha.update(data, length);
	sha.digest(hash);
}

Hash::Hash(const Hash& other)
{
	assign(other);
}

void Hash::assign(const Hash& other)
{
	hash = other.hash;
}

bool Hash::equals(const Hash& other) const
{
	//TODO: examine if there's anything to gain from a memcmp or manual native-integer-size based compare.
	return hash == other.hash;
}

std::string Hash::toString() const
{
	static const char *hexnibbles = "0123456789abcdef";

	std::string temp;
	temp.reserve(hash.size() * 2);
	for(auto val : hash) 
	{
		temp.push_back( hexnibbles[(val >> 4) & 0xF] );
		temp.push_back( hexnibbles[val & 0xF] );
	}

	return temp;
}

static int fromHexNibble(char nibble)
{
	if(nibble >= '0' && nibble <= '9') {
		return nibble - '0';
	}

	nibble |= 0x20; // converts upper- to lowercase
	if ( nibble >= 'a' && nibble <= 'f' ) {
		return (nibble - 'a' + 10);
	}

	return -1;
}

Hash Hash::fromHexString(const std::string& input)
{
	std::array<uint8_t, hashlength> blob = { 0 };

	if(input.length() != hashlength*2)
	{
		throw std::runtime_error("hashstring must be 64 hexadecimal characters");
	}

	for(int i=0; i<hashlength; ++i)
	{
		int byte =	(fromHexNibble(input[i*2 + 0]) << 4) |
					(fromHexNibble(input[i*2 + 1]));
		if ( static_cast<unsigned>(byte) > 0xFF) {
			throw std::runtime_error("hashstring must be 64 hexadecimal characters");
		}
		blob[i] = byte & 0xFF;
	}

	Hash temp;
	temp.hash = blob;
	return temp;
}



// strips out the dash in a DDMMYY-XXXX string. Assumes well formed input.
static std::string stripdash(std::string input)
{
	assert( input.length() == 11 );
	assert( input[6] == '-' );

	return input.substr(0, 6).append(input.substr(7, 4));
}

/******************************************************************************
* determines if a CPR string is conforms to DDMMYY-XXXX or DDMMYYXXXX
*
* returns true if cpr is valid
******************************************************************************/
bool isValidCpr(const std::string& cpr)
{
	if(cpr.length() == 10) {
		// DDMMYYXXXX - verify the string is all numeric.
		//TODO: verify the datepart is a valid date? not entirely trivial, given
		//that the yearpart is *possibly* a special encoding. For now, we simply
		//verify the string is entirely numeric.
		return cpr.find_first_not_of("0123456789") == cpr.npos;
		// return std::none_of(cpr.begin(), cpr.end(), [](char  x) { !std::isdigit(x); } );
	} else
	if(cpr.length() == 11) {
		// DDMMYY-XXXX form - strip '-' and verify recursively.
		return (cpr[6] == '-') ?
			isValidCpr( stripdash(cpr) ) :
			false;
	} else {
		return false;
	}
}

/******************************************************************************
* returns a Hash from a CPR string (DDMMYY-XXXX and DDMMYYXXXX both supported)
*
* It's the client's resposibility to ensure the input cpr is well-formed.
******************************************************************************/
Hash hashFromCpr(const std::string& cpr)
{
	assert( isValidCpr(cpr) );

	if(cpr.length() == 10) {
		return Hash( cpr.data(), 10 );
	} else {
		return Hash( stripdash(cpr).data(), 10 );
	}
}



class StopWatchImpl
{
public:
	typedef std::chrono::high_resolution_clock clock_t;

	clock_t clock;
	clock_t::time_point start, end;
};

StopWatch::StopWatch() : impl(new StopWatchImpl)
{
	reset();
}

StopWatch::~StopWatch()
{
	delete impl;
}

void StopWatch::reset()
{
	impl->start = impl->start.max();
	impl->end = impl->end.min();
}

void StopWatch::start()
{
	impl->start = impl->clock.now();
	impl->end = impl->end.min();
}

void StopWatch::stop()
{
	impl->end = impl->clock.now();
}

uint64_t StopWatch::getMilli() const
{
	StopWatchImpl::clock_t::time_point end;

	if(impl->end == impl->end.min() ) {
		// clock is "still running"
		end = impl->clock.now();
	} else {
		end = impl->end;
	}

	return std::chrono::duration_cast<std::chrono::milliseconds> ( end - impl->start).count();
}

std::string StopWatch::getFriendly() const
{
	//TODO: hhmmss.uu
	return getMilli() + "ms";
}












// lookup table defs and helper function forward declaration.
typedef std::array<char[4], days_per_year> lookup_t;									// (day,month) -> DDMM chars (not string, no NUL!)
static lookup_t generateDayMonthLookupTable();
static const lookup_t g_dayMonthLookup = generateDayMonthLookupTable();

void runpermutations(uint32_t start, uint32_t len, bool exhaustive, std::function<bool(const char*)> func)
{
	char cpr[11] = {};	// DDMMYYXXXX + NUL byte

	for(unsigned iter = start; iter < start+len; ++iter)
	{
		for(unsigned dayAndMonth=0; dayAndMonth<days_per_year; ++dayAndMonth)
		{
			memcpy( &cpr[0], g_dayMonthLookup[dayAndMonth], sizeof(g_dayMonthLookup[0]) );	// first four chars: DDMM
			base10fixWidthStr<6>(&cpr[4], iter);											// then follows the [0-999999]

			if( func(cpr) && !exhaustive ) {
				return;
			}
		}
	}
}

static lookup_t generateDayMonthLookupTable()
{
	static const std::array<unsigned, 12> days_per_month = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	lookup_t result = { 0 };
	// Initialize 'result' to avoid (spurious, since we fill it completely below?) analyzer warning about using it
	// uninitialized at the return statement. Speed hit is negligible anyway, especially since this is one-time init.

	char buf[4];
	unsigned index = 0;
	for(unsigned month=0; month<days_per_month.size(); ++month)
	{
		for(unsigned day=0; day<days_per_month[month]; ++day)
		{
			assert(index < days_per_year);

			base10fixWidthStr<2>(&buf[0], day + 1);
			base10fixWidthStr<2>(&buf[2], month + 1);
			memcpy( &result[index++], buf, sizeof(result[index]) );
		}
	}

	assert( index == days_per_year );
	return result;
}

} // namespace cprfun
