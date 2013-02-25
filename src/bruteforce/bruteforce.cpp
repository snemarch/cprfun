#include "stdafx.h"
#include <cstdint>
#include <cstdio>

#include <core/core.h>

using namespace cprfun;
using namespace std;

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
