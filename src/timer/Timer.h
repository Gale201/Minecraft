#pragma once
#include <chrono>
#include <iostream>

namespace Core
{
	struct Timer
	{
		float& resultTime;
		std::chrono::steady_clock::time_point startTime;

		Timer(float& resultTime) : resultTime(resultTime), startTime(std::chrono::steady_clock::now()) {}

		~Timer()
		{
			auto end = std::chrono::steady_clock::now();
			auto ms = std::chrono::duration<float, std::milli>(end - startTime).count();
			resultTime = ms;
		}
	};
}
