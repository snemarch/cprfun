#include "stdafx.h"
#include <cstdint>
#include <cstdio>

#include <core/core.h>

using namespace cprfun;
using namespace std;

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


int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		puts("bruteforce [numthreads] [sha256-hash] - tries to find a CPR number that matches your hash.");
		return 0;
	}

	/*
	try
	{
		Hash targetHash = Hash::fromHexString(argv[2]);
	}
	catch(const runtime_error &e)
	{
		puts( e.what() );
		return -1;
	}
	*/

	const string targetCpr("3112999999");
	const Hash targetHash = hashFromCpr(targetCpr);
	const unsigned numThreads = 2;

	bruteforce(targetHash);

	return 0;
}
