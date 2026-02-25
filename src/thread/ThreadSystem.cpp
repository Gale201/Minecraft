#include "ThreadSystem.h"

#include "../core/Log.h"

namespace Core
{
	void ThreadSystem::Initialize()
	{
		int threadCount = std::thread::hardware_concurrency() - 1;

		for (int i = 0; i < threadCount; i++)
			m_Workers.emplace_back(&ThreadSystem::WorkerLoop, this);

		m_MainThreadId = std::this_thread::get_id();
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

		if (std::this_thread::get_id() != m_MainThreadId)
		{
			LOG_ERROR("ParallelFor called from worker thread!");
		}

		auto state = std::make_shared<ParallelForState>();
		state->remaining = count;

		for (int i = 0; i < m_Workers.size(); i++)
		{
			Submit([state, function, count]() {
				while (true)
				{
					int i = state->index.fetch_add(1);
					if (i >= count)
						break;

					try
					{
						function(i);
					}
					catch (const std::exception& e)
					{
						LOG_ERROR("Thread exception: %s\n", e.what());
						__debugbreak();
					}

					{
						std::lock_guard<std::mutex> lock(state->waitMutex);
						state->remaining--;
						if (state->remaining == 0) state->waitCV.notify_one();
					}
				}
				});
		}

		std::unique_lock lock(state->waitMutex);
		state->waitCV.wait(lock, [&]() { return state->remaining == 0; });
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

			try {
				job();
			}
			catch (const std::exception& e)
			{
				LOG_ERROR("Thread exception: %s\n", e.what());
				__debugbreak();
			}
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
}
