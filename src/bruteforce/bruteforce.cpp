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

void runpermutations(uint32_t start, uint32_t len, bool exhaustive, std::function<bool(const string&)> func)
{
	static const std::array<unsigned, 12> days_per_month = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

// 	int total = accumulate(days_per_month.begin(), days_per_month.end(), 0, [](int accu, int next) { return accu+next; } );
// 	printf("days sum: %d\n", total);

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
		targetIterations * 366, targetCpr.c_str(), targetHash.toString().c_str() );

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
		puts("target CPR scanning not implemented yet");

	}

	/*
	const Hash targetHash = hashFromCpr("1510017321");	// 4aa7a49b7bed8eda47a0fbe7d0cc428d1fcdc5161774ec73be55530f63243b5a
	printf( "Scanning for hash %s\n", targetHash.toString().c_str() );
	
	runpermutations(0, 10000, true, [=](const string& cpr) {
		Hash currentHash = hashFromCpr(cpr);
		if(currentHash == targetHash)
		{
			printf( "Got a match! CPR == %s\n", cpr.c_str() );
			return true;
		}
		return false;
	});
	*/


	return 0;
}