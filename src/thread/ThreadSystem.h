#pragma once
#include <functional>
#include <thread>
#include <queue>
#include <mutex>

class ThreadSystem
{
public:
	void Initialize();
	void Shutdown();

	void Submit(std::function<void()> job);

	void ParallelFor(int count, std::function<void(int)> function);

private:
	void WorkerLoop();

	std::vector<std::thread> m_Workers;
	std::queue<std::function<void()>> m_Jobs;

	std::mutex m_Mutex;
	std::condition_variable m_Condition;
	bool m_Shutdown = false;

	std::thread::id m_MainThreadId;
};
