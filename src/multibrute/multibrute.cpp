#include "stdafx.h"
#include <atomic>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

#include <core/core.h>
#include <core/SHA256.h>

// hash(3112999999) => 7c1bb9f1d19393fdbcb04537a679d661180f31166b8ecb21135711c84883914e	// full keyspace
// hash(3112009999) => b200598b0dcc2848b488c70bb3d8261702977ded093fd82ecf9911bf3852e077	// small search, early-out

using namespace cprfun;
using namespace std;

struct threadparam_t 
{
	const Hash				*targetHash;
	atomic_uint_fast32_t	progress;
	const uint_fast32_t		start;
	const uint_fast32_t		count;
	const unsigned			threadnum;
};

//TODO: get rid of the globals.
bool debug = false;
vector<unique_ptr<threadparam_t>> params;
atomic<bool> g_stopSearching(false);

static void bruteforce(threadparam_t *param)
{
	if(debug) {
		cout << "Thread " << param->threadnum <<
				": scanning for hash " << param->targetHash->toString() 	<<
				", range " << param->start << "-" << (param->start + param->count) << endl;
	}

	sha256 sha;
	uint_fast32_t count;
	runpermutations(param->start, param->count, false, [&](const char *cpr) -> bool {
		// Ugh, this is a bit hacky... so, the "count" param is the number of years the thread processes,
		// so we need to divide by 366 (the number of always-with-leap days in a year) to do percentage
		// calculations... we might as well throw in a bit of a micro-optimization: atomics are somewhat
		// expensive to use, so only update every Xxx iterations.
		if ((count++ % 366'000) == 0) {
			param->progress += 1'000;
		}

		if(const Hash currentHash(sha, cpr, 10); currentHash == *param->targetHash)
		{
			cout << endl << "Thread " << param->threadnum << ": got a match! CPR == " << cpr << endl;
			g_stopSearching = true;
		}
		return g_stopSearching;
	});

	if (debug) {
		cout << "Thread " << param->threadnum << ": exhausted keyspace slice" << endl;
	}
}

static vector<thread> createAndLaunchWorkers(uint8_t numThreads, const Hash& targetHash);
static void waitForWorkers(vector<thread>& workers);
static void reportProgress(const vector<unique_ptr<threadparam_t>> & vector);

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
		reportProgress(params);
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
	for(auto t=0; t<numThreads; ++t)
	{
		auto& p = params.emplace_back(make_unique<threadparam_t>(
			&targetHash,
			0,
			t * slicesize,
			min(slicesize, (keyspace - (t * slicesize))),
			t
		));

		workers.emplace_back( bruteforce, p.get() );
	}

	return workers;
}

static void reportProgress(const vector<unique_ptr<threadparam_t>>& vector)
{
	while (!g_stopSearching)
	{
		cout << "\r";
		for(auto& t : vector)
		{
			cout << fixed << setprecision(1) << (t->progress * 100.0 / t->count) << " ";
		}
		cout << flush;
		this_thread::sleep_for(chrono::milliseconds(200));
	}
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
