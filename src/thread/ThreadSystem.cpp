#include "ThreadSystem.h"

#include "../core/Log.h"
#include <assert.h>

namespace Core
{
	thread_local int g_ThreadIndex = -1;

	void ThreadSystem::Initialize()
	{
		unsigned int hw = std::thread::hardware_concurrency();
		int threadCount = hw > 1 ? static_cast<int>(hw) - 1 : 0;

		for (int i = 0; i < threadCount; i++)
			m_Workers.emplace_back([this, i]()
				{
					g_ThreadIndex = i;
					WorkerLoop();
				});

		g_ThreadIndex = threadCount;
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

	void ThreadSystem::ParallelFor(int count, const std::function<void(int)>& function)
	{
		assert(std::this_thread::get_id() == m_MainThreadId);
		if (count <= 0) return;

		auto state = std::make_shared<ParallelForState>();
		state->remaining = count;

		auto run = [state, &function, count]() {
			for (int i; (i = state->index.fetch_add(1)) < count; )
			{
				try
				{
					function(i);
				}
				catch (const std::exception& e)
				{
					LOG_ERROR("Thread exception: %s\n", e.what());
					__debugbreak();
				}
				std::lock_guard lock(state->waitMutex);
				if (--state->remaining == 0) state->waitCV.notify_one();
			}
		};

		for (size_t w = 0; w < m_Workers.size(); w++)
		{
			Submit(run);
		}
		run();

		std::unique_lock lock(state->waitMutex);
		state->waitCV.wait(lock, [&] { return state->remaining == 0; });
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

	int ThreadSystem::GetCurrentThreadIndex() const { return g_ThreadIndex; }
}
