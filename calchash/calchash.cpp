#include "stdafx.h"
#include <cstdio>
#include <string>

#include <core/core.h>

using namespace cprfun;

int main(int argc, char *argv[])
{
	if(argc < 2) {
		puts("usage: calchash [cprnumber], where cprnumber is in DDMMYYXXXX or DDMMYY-XXXX form.");
		return 0;
	}

	if(!isValidCpr(argv[1])) {
		puts("error: invalid cprnumber");
		return -1;
	}

	Hash cprHash = hashFromCpr( argv[1] );
	printf("hash(%s) => %s\n", argv[1], cprHash.toString().c_str() );

	return 0;
}
