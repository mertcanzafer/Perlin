#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <functional>
#include <memory>

namespace PERLIN_UTIL {

	class ThreadPool 
	{
	public:
		explicit ThreadPool(const size_t numThreads = std::thread::hardware_concurrency());

		ThreadPool(const ThreadPool&) = delete;
		ThreadPool& operator=(const ThreadPool&) = delete;

		/* Accept any callable object which can be a function, lambda or functor.
		 * Returns a future that will hold the result of the callable.
		 * The return type is deduced using std::invoke_result_t.
		 * 
		 * Example usage:
		 * auto future = threadPool.enqueue([](int x) { return x * 2; }, 5);
		 * int result = future.get(); // Will return 10
		*/
		template<typename Func, typename... Args> 
		auto enqueue(Func&& f, Args&&... args) 
			-> std::future<std::invoke_result_t<Func,Args ...>>
		{
			using ReturnType = std::invoke_result_t<Func, Args ...>;

			auto pTask = std::make_shared<std::packaged_task<ReturnType()>>(
				std::bind(std::forward<Func>(f), std::forward<Args>(args)...)
			);

			{
				std::lock_guard<std::mutex> lock(m_QueueMutex);
				if (m_Stop) 
				{
					throw std::runtime_error("enqueue on stopped ThreadPool");
				}

				m_Tasks.emplace([pTask]()
					{
						*pTask();
					});
			}

			m_Condition.notify_one(); // Notify one waiting thread
			return pTask->get_future(); // Return the future associated with the task
		}

		~ThreadPool();
	private:
		std::vector<std::thread> m_Workers;
		std::queue<std::function<void()>> m_Tasks;
		std::mutex m_QueueMutex;
		std::condition_variable m_Condition;
		std::atomic<bool> m_Stop;
	};
}