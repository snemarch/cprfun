#pragma once
#ifndef cprfun__workqueue_h
#define cprfun__workqueue_h

#include "stdafx.h"
#include <vector>
#include <thread>
#include <semaphore>
#include <array>
#include <atomic>

namespace cprfun {

template <typename T>
class WorkQueue {
public:
	WorkQueue(size_t chunkSize) :
		buffers{ std::make_unique<std::vector<T>>(), std::make_unique<std::vector<T>>() },
		bufferAvailable(1), workerReady(0), chunkSize(chunkSize), running(true) {

		buffers[Buffers::FRONT]->reserve(chunkSize);
		buffers[Buffers::BACK]->reserve(chunkSize);
	}

	// Producer adds a single item and flushes when the chunk is full
	template <typename... Args>
	void produce(Args&&... args) {
		buffers[Buffers::FRONT]->emplace_back(std::forward<Args>(args)...);

		if (buffers[Buffers::FRONT]->size() >= chunkSize) {
			flush();
		}
	}

	void flush(bool shutdown = false) {
		if (buffers[Buffers::FRONT]->empty() && !shutdown) return;

		bufferAvailable.acquire();

		buffers[Buffers::FRONT].swap(buffers[Buffers::BACK]);
		std::atomic_thread_fence(std::memory_order_release);

		workerReady.release();
	}

	// Consumer retrieves the back buffer reference
	std::vector<T> *consume() {
		workerReady.acquire();

		if (!running && buffers[Buffers::BACK]->empty()) {
			return nullptr;
		}

		std::atomic_thread_fence(std::memory_order_acquire);
		return buffers[Buffers::BACK].get();
	}

	// Consumer signals that it has finished processing
	void done() {
		buffers[Buffers::BACK]->clear();
		bufferAvailable.release();
	}

	// Signals shutdown and ensures the worker exits
	void shutdown() {
		running = false;
		flush(true);  // Ensure the worker wakes up for final check
	}

private:
	std::array<std::unique_ptr<std::vector<T>>, 2> buffers;
	std::binary_semaphore bufferAvailable;
	std::binary_semaphore workerReady;
	const size_t chunkSize;
	std::atomic<bool> running;
	enum Buffers { FRONT = 0, BACK = 1 };
};

}

#endif // cprfun__workqueue_h
