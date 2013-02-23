#include "stdafx.h"
#include <algorithm>
#include <cassert>
#include <cctype>

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
* Throws somethingsomething on error
******************************************************************************/
Hash hashFromCpr(const std::string& cpr)
{
	if(!isValidCpr(cpr)) {
		throw std::runtime_error("nope, chuck testa");
	}

	if(cpr.length() == 10) {
		return Hash( cpr.data(), 10 );
	} else {
		return Hash( stripdash(cpr).data(), 10 );
	}
}



}
