#pragma once

#include <cstdarg>
#include <cstdio>

#define LOG_INFO(...) Log(LogLevel::INFO, __VA_ARGS__)
#define LOG_WARNING(...) Log(LogLevel::WARNING, __VA_ARGS__)
#define LOG_ERROR(...) Log(LogLevel::ERROR, __VA_ARGS__)

enum class LogLevel
{
	INFO = 0,
	WARNING,
	ERROR
};

inline void Log(LogLevel level, const char* format, ...)
{
	const char* prefix = "";

	switch (level)
	{
	case LogLevel::INFO:
		prefix = "[INFO] ";
		break;
	case LogLevel::WARNING:
		prefix = "[WARNING] ";
		break;
	case LogLevel::ERROR:
		prefix = "[ERROR] ";
		break;
	default:
		break;
	}

	std::printf("%s", prefix);

	va_list args;
	va_start(args, format);
	std::vprintf(format, args);
	va_end(args);

	std::printf("\n");
}