#pragma once
#include <chrono>
#include <iostream>

namespace Core
{
	struct Timer
	{
		const char* name;
		std::chrono::steady_clock::time_point startTime;

		Timer(const char* name) : name(name), startTime(std::chrono::steady_clock::now()) {}

		~Timer()
		{
			auto end = std::chrono::steady_clock::now();
			auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - startTime).count();
			std::cout << name << ": " << us << " us" << std::endl;
		}
	};
}
