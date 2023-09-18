#pragma once
#include <functional>

namespace JobSystem {
	void Initialize();
	// Add a job to execute asynchronously. Any idle thread will execute this job.
	void Execute(const std::function<void()>& job);
	// Divide a job onto multiple jobs and execute in parallel.
	void Dispatch(uint32_t jobCount, uint32_t groupSize, const std::function<void(JobDispatchArgs)>& job);
	// Check if any threads are working currently or not
	bool IsBusy();
	// Wait until all threads become idle
	void Wait();
}