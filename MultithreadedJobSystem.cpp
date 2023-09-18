#include <iostream>
#include <thread>
#include <mutex>
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <deque>

#include "MultithreadedJobSystem.h"

// Following https://wickedengine.net/2018/11/24/simple-job-system-using-standard-c/

// TODO: Try to implement the updated version in the WickedEngine after you finish the tutorial

struct JobDispatchArgs {
	uint32_t jobIndex;
	uint32_t groupIndex;
};

struct context
{
	std::atomic<uint32_t> counter{ 0 };
};

struct Job {
	std::function<void(JobDispatchArgs)> task;
	context* ctx;
	uint32_t groupID;
	uint32_t groupJobOffset;
	uint32_t groupJobEnd;
};

// Fixed size very simple thread safe ring buffer: Necessary and can't use deque instead of a custom
// class/struct for ring bufferbc you can't
// compare functions to see if they're equal so you can't check if a certain job is already in the 
// deque. However, you can substitute capacity for deque and you don't need to return bool for push_back
// since there wouldn't be a limit on how many jobs ThreadSafeRingBuffer can take
struct JobQueue
{
	std::deque<Job> queue;
	std::mutex lock;
	
	inline bool push_back(const Job& item)
	{
		lock.lock();
		queue.push_back(item);
	}

	// Get an item if there are any
	//  Returns true if succesful
	//  Returns false if there are no items
	inline bool pop_front(Job& item)
	{
		lock.lock();
		if (queue.empty())
		{
			return false;
		}
		item = std::move(queue.front());
		queue.pop_front();
		return true;
	}
};

namespace JobSystem {
	uint32_t numThreads = 0;
	JobQueue jobPool;
	std::condition_variable wakeCondition;
	std::mutex wakeMutex;
	uint64_t currentLabel = 0;
	std::atomic<uint64_t> finishedLabel;

	void Initialize() {
		finishedLabel.store(0);
		numThreads = std::thread::hardware_concurrency();
		printf("The number of cores in this machine is: %d \n", numThreads);

		for (uint32_t threadID = 0; threadID < numThreads; ++threadID) {
			std::thread worker([] {
				std::function<void()> job;

				while (true) {
					if (jobPool.pop_front(job)) {
						job();
						finishedLabel.fetch_add(1);
					}
					else {
						// no job, put thread to sleep
						std::unique_lock<std::mutex> lock(wakeMutex);
						wakeCondition.wait(lock);
					}
				}
			});
				
			worker.detach();
		}
	}

	inline void poll() {
		wakeCondition.notify_one(); // wake one worker thread
		std::this_thread::yield();
	}

	bool IsBusy() {
		// Whenever the main thread label is not reached by the workers, it indicates that some worker is still alive
		return finishedLabel.load() < currentLabel;
	}

	void Wait() {
		while (IsBusy()) { poll(); }
	}

	void Execute(const std::function<void()>& job) {
		// main thread label state is updated
		currentLabel += 1; 

		// Try to push a new job until it is pushed successfully:
		jobPool.push_back(job);
		while (!jobPool.push_back(job)) { poll(); }
		wakeCondition.notify_one();
	}

	// Divide a job onto multiple jobs and execute in parallel.
	//  jobCount    : how many jobs to generate for this task.
	//  groupSize   : how many jobs to execute per thread. Jobs inside a group execute serially. 
	//  It might be worth to increase for small jobs
	//void Dispatch(uint32_t jobCount, uint32_t groupSize, const std::function<void(JobDispatchArgs)> job) {
	//	if (jobCount == 0 || groupSize == 0) {
	//		return;
	//	}

	//	// Calculate the amount of job groups to dispatch
	//	const uint32_t groupCount = (jobCount + groupSize - 1) / groupSize;
	//	currentLabel += groupCount;

	//	for (uint32_t groupIndex = 0; groupIndex < groupCount; ++groupIndex) {
	//		auto jobGroup = [jobCount, groupSize, groupIndex, job]() {
	//			const uint32_t groupJobOffset = groupIndex * groupSize;
	//			const uint32_t groupJobEnd = std::min(groupJobOffset + groupSize, jobCount);

	//			JobDispatchArgs args;
	//			args.groupIndex = groupIndex;

	//			for (uint32_t i = groupJobOffset; i < groupJobEnd; ++i) {
	//				args.jobIndex = i;
	//				job(args);
	//			}
	//		};

	//		// Try to push new job until pushed successfully:
	//		while (!jobPool.push_back(jobGroup)) { poll(); }

	//		wakeCondition.notify_one(); // wake one thread
	//	}

	//}
}