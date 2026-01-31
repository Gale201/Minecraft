#pragma once
#include <cstdint>
#include <chrono>

struct AccumTimer
{
	uint64_t& acc;
	std::chrono::steady_clock::time_point startTime;

	AccumTimer(uint64_t& acc) : acc(acc), startTime(std::chrono::steady_clock::now()) {}

	~AccumTimer()
	{
		auto end = std::chrono::steady_clock::now();
		auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - startTime).count();
		acc += us;
	}
};
