#include "ThreadSystem.h"

void ThreadSystem::Initialize()
{
	int threadCount = std::thread::hardware_concurrency();

	for (int i = 0; i < threadCount; i++)
		m_Workers.emplace_back(&ThreadSystem::WorkerLoop, this);
}

void ThreadSystem::Shutdown()
{
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		m_Shutdown = true;
	}

	m_Condition.notify_all();

	for (std::thread& t : m_Workers)
	{
		if (t.joinable()) t.join();
	}
}

void ThreadSystem::ParallelFor(int count, std::function<void(int)> function)
{
	if (count == 0) return;

	std::atomic<int> index = 0;
	std::atomic<int> remaining = count;

	std::mutex waitMutex;
	std::condition_variable waitCV;

	for (int i = 0; i < m_Workers.size(); i++)
	{
		Submit([&, function, count]() {
			while (true)
			{
				int i = index.fetch_add(1);
				if (i >= count)
					break;

				function(i);

				if (remaining.fetch_sub(1) == 1)
				{
					std::lock_guard<std::mutex> lock(waitMutex);
					waitCV.notify_one();
				}
			}
		});
	}

	std::unique_lock lock(waitMutex);
	waitCV.wait(lock, [&]() { return remaining == 0;  });
}

void ThreadSystem::WorkerLoop()
{
	while (true)
	{
		std::function<void()> job;

		{
			std::unique_lock lock(m_Mutex);
			m_Condition.wait(lock, [this]() { return !m_Jobs.empty() || m_Shutdown; });
			if (m_Shutdown) 
				return;

			job = std::move(m_Jobs.front());
			m_Jobs.pop();
		}

		job();
	}
}

void ThreadSystem::Submit(std::function<void()> job)
{
	{
		std::lock_guard lock(m_Mutex);
		m_Jobs.push(std::move(job));
	}

	m_Condition.notify_one();
}
