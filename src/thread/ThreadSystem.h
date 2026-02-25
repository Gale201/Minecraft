#pragma once
#include <functional>
#include <thread>
#include <queue>
#include <mutex>

namespace Core
{
	class ThreadSystem
	{
	public:
		void Initialize();
		void Shutdown();

		void Submit(std::function<void()> job);

		void ParallelFor(int count, std::function<void(int)> function);
		
	private:
		struct ParallelForState
		{
			std::atomic<int> index{ 0 };
			int remaining;
			std::mutex waitMutex;
			std::condition_variable waitCV;
		};

	private:
		void WorkerLoop();

		std::vector<std::thread> m_Workers;
		std::queue<std::function<void()>> m_Jobs;

		std::mutex m_Mutex;
		std::condition_variable m_Condition;
		bool m_Shutdown = false;

		std::thread::id m_MainThreadId;
	};
}
