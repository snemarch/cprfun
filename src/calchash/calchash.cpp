#include "stdafx.h"
#include <iostream>
#include <string>

#include <core/core.h>

using namespace cprfun;
using namespace std;

int main(int argc, char *argv[])
{
	if(argc < 2) {
		cout << "usage: calchash [cprnumber], where cprnumber is in DDMMYYXXXX or DDMMYY-XXXX form." << endl;
		return 0;
	}

	if(!isValidCpr(argv[1])) {
		cout << "error: invalid cprnumber" << endl;
		return -1;
	}

	Hash cprHash = hashFromCpr( argv[1] );
	cout << "hash(" << argv[1] << ") => " << cprHash.toString() << endl;

	return 0;
}
