#include "Clock.h"

#include "SDL.h"

namespace Core
{
	Clock::Clock(int targetFps)
	{
		m_TargetFrameTime = 1.0f / (float)targetFps;
		m_LastTime = SDL_GetPerformanceCounter();
		m_Frequency = SDL_GetPerformanceFrequency();
	}

	void Clock::Tick()
	{
		uint64_t currentTime = SDL_GetPerformanceCounter();
		m_ElapsedTime = currentTime - m_LastTime;

		float frameTime = (float)m_ElapsedTime / (float)m_Frequency;
		if (frameTime < m_TargetFrameTime)
		{
			float sleepTime = m_TargetFrameTime - frameTime;
			if (sleepTime > 0.001f)
				SDL_Delay((uint32_t)(sleepTime * 1000.0f));

			currentTime = SDL_GetPerformanceCounter();
			m_ElapsedTime = currentTime - m_LastTime;
			frameTime = (float)m_ElapsedTime / (float)m_Frequency;
		}

		m_LastTime = currentTime;

		m_DeltaTime = frameTime;
		m_Fps = frameTime > 0.0f ? 1.0f / frameTime : 0.0f;
	}
}
