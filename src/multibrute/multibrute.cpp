#include "stdafx.h"
#include <atomic>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <thread>
#include <vector>

#include <core/core.h>

// hash(3112999999) => 7c1bb9f1d19393fdbcb04537a679d661180f31166b8ecb21135711c84883914e	// full keyspace
// hash(3112009999) => b200598b0dcc2848b488c70bb3d8261702977ded093fd82ecf9911bf3852e077	// small search, early-out

using namespace cprfun;
using namespace std;

atomic<bool> g_stopSearching(false);

struct threadparam_t 
{
	const Hash	*targetHash;
	uint32_t	start;
	uint32_t	count;
	int			threadnum;
};

static void bruteforce(const threadparam_t& param)
{
	cout << "Thread " << param.threadnum << 
			": scanning for hash " << param.targetHash->toString() 	<< 
			", range " << param.start << "-" << (param.start + param.count) << endl;

	runpermutations(param.start, param.count, false, [=](const char *cpr) -> bool {
		Hash currentHash(cpr, 10);
		if(currentHash == *param.targetHash)
		{
			cout << "Thread " << param.threadnum << ": got a match! CPR == " << cpr << endl;
			g_stopSearching = true;
		}
		return g_stopSearching;
	});

	cout << "Thread " << param.threadnum << ": exhausted keyspace slice" << endl;
}

static vector<thread> createAndLaunchWorkers(uint8_t numThreads, const Hash& targetHash);
static void waitForWorkers(vector<thread>& workers);

int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		cout << "bruteforce [numthreads] [sha256-hash] - tries to find a CPR number that matches your hash." << endl <<
				"numthreads must be in [1..255] range" << endl;
		return 0;
	}

	try {
		const int numThreads = stoi(argv[1]);
		if(1 > numThreads || numThreads > 255) {
			throw out_of_range("numthreads must be in [1..255] range");
		}

		const Hash targetHash = Hash::fromHexString(argv[2]);

		vector<thread> threads = createAndLaunchWorkers(static_cast<uint8_t>(numThreads), targetHash);
		cout <<  "Worker threads launched, waiting for completion - this may take a while!" << endl;
		waitForWorkers(threads);
	}
	catch(const invalid_argument& ex) {
		cout << "Invalid argument: " << ex.what() << endl;
		return -1;
	}
	catch(const out_of_range& ex) {
		cout << "Out of range: " << ex.what() << endl;
		return -1;
	}
	catch(const runtime_error& ex) {
		cout << "Runtime error: " << ex.what() << endl;
		return -1;
	}

	return 0;
}

static vector<thread> createAndLaunchWorkers(uint8_t numThreads, const Hash& targetHash)
{
	constexpr uint32_t keyspace = 1'000'000;
	const auto slicesize = static_cast<unsigned>( ceil(static_cast<double>(keyspace) / numThreads) );

	vector<thread> workers;
	for(uint8_t t=0; t<numThreads; ++t)
	{
		threadparam_t param;

		param.start = t * slicesize;
		param.count = min( slicesize, (keyspace - param.start) );
		param.targetHash = &targetHash;
		param.threadnum = t;

		workers.push_back( thread(bruteforce, param) );
	}
	return workers;
}

static void waitForWorkers(vector<thread>& workers)
{
	for(auto& thread : workers)
	{
		if( thread.joinable() )
		{
			thread.join();
		}
	}
}
