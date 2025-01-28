#include "stdafx.h"
#include <cstring>
#include <cstdint>
#include <iostream>

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

	cout << "Benchmarking - expecting to reach " << (targetIterations * days_per_year) <<
		" permutations, cpr [" << targetCpr << "] and hash " << targetHash.toString() << endl;

	StopWatch sw;
	sw.start();
	uint32_t numPermutations = 0;
	runpermutations(0, targetIterations, true, [&](const char *cpr) -> bool {
		++numPermutations;
		Hash currentHash(cpr, 10);
		if(currentHash == targetHash)
		{
			cout << "After " << numPermutations << " iterations: found cpr [" <<
					cpr << "] with hash [" << currentHash.toString() << "]" << endl;

			return true;
		}
		return false;
	});
	sw.stop();

	cout << "Runtime " << sw.getMilli() << "ms, " << (static_cast<uint64_t>(numPermutations)*1000)/sw.getMilli() << " hashops/sec" << endl;
}

void bruteforce(const Hash& targetHash)
{
	cout << "Scanning for hash " << targetHash.toString() << endl;
	
	unsigned iterations = 0;
	runpermutations(0, (100*10000) - 1, true, [&](const char *cpr) -> bool {
		if( (iterations++ % 3660000) == 0 )
		{
			cout << "reached " << cpr << endl;
		}

		Hash currentHash(cpr, 10);
		if(currentHash == targetHash)
		{
			cout << "Got a match! CPR == " << cpr << endl;
			return true;
		}
		return false;
	});
}

int main(int argc, char* argv[])
{
	if(argc < 2)
	{
		cout << "bruteforce [sha256-hash] - tries to find a CPR number that matches your hash" << endl <<
				"\tAlternatively, specify --benchmark to run 50k iterations for an estimate of hashops/sec." << endl;
		return 0;
	}

	if( string(argv[1]) == "--benchmark" ) {
		benchmark(50'000);
	} else {
		try
		{
			Hash targetHash = Hash::fromHexString(argv[1]);
			bruteforce(targetHash);
		}
		catch(const runtime_error &e)
		{
			cout << e.what() << endl;
			return -1;
		}
	}

	return 0;
}
