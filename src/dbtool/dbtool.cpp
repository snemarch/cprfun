#include "stdafx.h"
#include <iomanip>
#include <iostream>
#include <string>

#include <core/core.h>
#include "hashstore.h"

using namespace cprfun;
using namespace std;

static void hs_create(const std::string& hsfilename)
{
	cout << "Creating hashstore - this will take a while..." << endl;

	try
	{
		HashStore store( HashStore::createNew(hsfilename) );
		unsigned iterations = 0;
		StopWatch timer;
		timer.start();
//		runpermutations(0, ( 10*10000), true, [&](const char *cpr) -> bool {
		runpermutations(0, (100*10000), true, [&](const char *cpr) -> bool {
			if( (iterations++ % 366000) == 0 )
			{
				cout << "reached " << cpr << " (" << fixed << setprecision(3) << ((iterations*100.0)/360000000.0) << "% done)" << endl;
			}

			Hash currentHash(cpr, 10);
			store.put(currentHash, cpr);
			return false; // keep on truckin'
		});
		timer.stop();
		cout << "done! - " << timer.getMilli() << "ms" << endl;
		cout << "build index - this will also take a while..." << endl;
		timer.reset();
		timer.start();
		store.buildIndex();
		timer.stop();
		cout << "done! - " << timer.getMilli() << "ms" << endl;
	}
	catch(const runtime_error& ex)
	{
		cout << "runtime error: " << ex.what() << endl;
	}
}

static void hs_lookup(const std::string& hsfilename, const std::string& hashstr)
{
	const Hash hash( Hash::fromHexString(hashstr) );

	HashStore store( HashStore::openExisting(hsfilename) );
	string cpr;
	if(store.tryGet(hash, &cpr))
	{
		cout << "found: " << cpr << endl;
	} else
	{
		cout << "sorry, hash not found in hashstore" << endl;
	}
}

int main(int argc, char* argv[])
{
	if(argc < 3)
	{
		cout << "Usage: dbtool [path/to/hashstore.db] [hashvalue] to lookup hashvalue in hashstore," << endl <<
				"\t or dbtool --create [path/to/hasthstore.db] to create lookup table (warning: will take a while!)" << endl;
		return 0;
	}

	if(string(argv[1]) == "--create")
	{
		hs_create(argv[2]);
	} else
	{
		hs_lookup(argv[1], argv[2]);
	}

	return 0;
}
