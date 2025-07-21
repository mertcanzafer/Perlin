#include "ThreadPool.h"

PERLIN_UTIL::ThreadPool::ThreadPool(const size_t numThreads)
	: m_Stop(false)
{
	for (size_t i = 0; i < numThreads; ++i)
	{
		m_Workers.emplace_back(
		
			[this]()
			{
				while (true)
				{
					std::function<void()> task;
					// Using a scoped lock to ensure thread safety
					{
						std::unique_lock lock(m_QueueMutex);
						m_Condition.wait(lock, [this] {return m_Stop || !m_Tasks.empty(); });
						if(m_Stop && m_Tasks.empty())
							return; // Exit if stopping and no tasks are left
						
						task = std::move(m_Tasks.front());

						m_Tasks.pop();
					}
					task(); // Execute the task
				}
			}
		);
	}
}

PERLIN_UTIL::ThreadPool::~ThreadPool()
{
	{
		std::lock_guard<std::mutex> lock(m_QueueMutex);
		m_Stop = true; // Signal all threads to stop
	}
	m_Condition.notify_all(); // Wake up all threads

	for (auto& worker : m_Workers)
	{
		if (worker.joinable())
		{
			worker.join(); // Wait for all threads to finish
		}
	}
}
