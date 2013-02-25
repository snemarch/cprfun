#include "stdafx.h"
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <functional>

#include <core/core.h>

using namespace cprfun;
using namespace std;

static const unsigned days_per_year = 366;										// to avoid dealing with leap years

// lookup table defs and helper function forward declaration.
typedef array<char[4], days_per_year> lookup_t;									// (day,month) -> DDMM chars (not string, no NUL!)
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

void benchmark(uint32_t targetIterations)
{
	char cprStrInput[10+1] = "3112";
	base10fixWidthStr<6>(&cprStrInput[4], targetIterations - 1);
	cprStrInput[10] = 0;

	const string targetCpr( cprStrInput );
	const Hash targetHash( hashFromCpr(targetCpr) );

	printf( "Benchmarking - expecting to reach %lu permutations, cpr [%s] and hash [%s]\n",
		targetIterations * days_per_year, targetCpr.c_str(), targetHash.toString().c_str() );

	StopWatch sw;
	sw.start();
	uint32_t numPermutations = 0;
	runpermutations(0, targetIterations, true, [&](const char *cpr) {
		++numPermutations;
		Hash currentHash(cpr, 10);
		if(currentHash == targetHash)
		{
			printf( "After %u iterations: found cpr [%s] with hash [%s] \n", numPermutations,
				cpr, currentHash.toString().c_str() );

			return true;
		}
		return false;
	});
	sw.stop();

	printf("Runtime %llu ms, %llu hashops/sec\n", sw.getMilli(), (static_cast<uint64_t>(numPermutations)*1000)/sw.getMilli() );
}

void bruteforce(const Hash& targetHash)
{
	printf( "Scanning for hash %s\n", targetHash.toString().c_str() );
	
	runpermutations(0, (100*10000) - 1, true, [=](const char *cpr) {
		if( (0 == memcmp( &cpr[0], "0101", 4)) &&
			(0 == memcmp( &cpr[6], "0000", 4)) )
		{
			printf("reached %s\n", cpr );
		}

		Hash currentHash(cpr, 10);
		if(currentHash == targetHash)
		{
			printf( "Got a match! CPR == %s\n", cpr );
			return true;
		}
		return false;
	});
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

int main(int argc, char* argv[])
{
	if(argc < 2)
	{
		puts("bruteforce [sha256-hash] - tries to find a CPR number that matches your hash");
		puts("\tAlternatively, specify --benchmark to run 10k iterations for an estimate of hashops/sec.");
		return 0;
	}

	if( string(argv[1]) == "--benchmark" ) {
		benchmark(20000);
	} else {
		try
		{
			Hash targetHash = Hash::fromHexString(argv[1]);
			bruteforce(targetHash);
		}
		catch(const runtime_error &e)
		{
			puts( e.what() );
			return -1;
		}
	}

	return 0;
}
