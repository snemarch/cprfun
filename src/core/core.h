#pragma once
#ifndef cprfun__core_h
#define cprfun__core_h

#include <array>
#include <cstdint>
#include <string>

namespace cprfun {

class Hash {
public:
	Hash();
	Hash(const void *data, size_t length);
	Hash(const Hash& other);

	void assign(const Hash& other);
	bool equals(const Hash& other) const;
	std::string toString() const;

	static Hash fromHexString(const std::string& input);

private:
	static const size_t hashlength = 256/8;
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



class StopWatchImpl;
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
	StopWatchImpl *impl;
};


} // namespace cprfun

#endif // cprfun__core_h
