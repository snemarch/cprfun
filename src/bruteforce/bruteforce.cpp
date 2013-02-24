#include "stdafx.h"
#include <array>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <numeric>
#include <sstream>

#include <core/core.h>

using namespace cprfun;
using namespace std;

static const unsigned days_per_year = 366;	// to avoid dealing with leap years

void runpermutations(uint32_t start, uint32_t len, bool exhaustive, std::function<bool(const string&)> func)
{
	static const std::array<unsigned, 12> days_per_month = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	// the following is hopelessly slow - that's on purpose so we have something to optimize later :-)
	for(unsigned iter = start; iter < start+len; ++iter)
	{
		for(unsigned month=0; month<days_per_month.size(); ++month)
		{
			for(unsigned day=0; day<days_per_month[month]; ++day)
			{
				stringstream ss;
				ss << setfill('0') << setw(2) << day+1 << setw(2) << month+1 << setw(6) << iter;
				if( func(ss.str()) && !exhaustive ) {
					return;
				}
			}
		}
	}
}

void benchmark(uint32_t targetIterations)
{
	stringstream format;
	format << "3112" << setfill('0') << setw(6) << targetIterations-1;
	const string targetCpr( format.str() );
	const Hash targetHash( hashFromCpr(targetCpr) );

	Hash foundHash;
	string foundCpr;

	printf( "Benchmarking - expecting to reach %lu permutations, cpr [%s] and hash [%s]\n",
		targetIterations * days_per_year, targetCpr.c_str(), targetHash.toString().c_str() );

	StopWatch sw;
	sw.start();
	uint32_t numPermutations = 0;
	runpermutations(0, targetIterations, true, [&](const string& cpr) {
		++numPermutations;
		Hash currentHash = hashFromCpr(cpr);
		if(currentHash == targetHash)
		{
			foundHash.assign(currentHash);
			foundCpr = cpr;
			return true;
		}
		return false;
	});
	sw.stop();

	printf( "After %u iterations: found cpr [%s] with hash [%s] \n", numPermutations,
		foundCpr.c_str(), foundHash.toString().c_str() );
	printf("Runtime %llu ms, %llu hashops/sec", sw.getMilli(), (static_cast<uint64_t>(numPermutations)*1000)/sw.getMilli() );
}

void bruteforce(const Hash& targetHash)
{
	printf( "Scanning for hash %s\n", targetHash.toString().c_str() );
	
	runpermutations(0, (100*10000) - 1, true, [=](const string& cpr) {
		if(( cpr.substr(0, 4) == "0101") && (cpr.substr(6, 4) == "0000") ) {
			printf("reached %s\n", cpr.c_str() );
		}
		Hash currentHash = hashFromCpr(cpr);
		if(currentHash == targetHash)
		{
			printf( "Got a match! CPR == %s\n", cpr.c_str() );
			return true;
		}
		return false;
	});
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
		benchmark(10000);
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
