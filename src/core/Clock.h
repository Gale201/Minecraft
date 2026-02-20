#pragma once

#include <cinttypes>

namespace Core
{
	class Clock
	{
	public:
		Clock(int targetFps);
		~Clock() = default;

		void Tick();
		float GetDeltaTime() const { return m_DeltaTime; }
		uint64_t GetElapsedTime() const { return m_ElapsedTime; }
		float GetTargetFps() const { return 1.0f / m_TargetFrameTime; }
		float GetFps() const { return m_Fps; }

	private:

		float m_TargetFrameTime;
		uint64_t m_LastTime;
		uint64_t m_Frequency;

		float m_DeltaTime = 0.0f;
		uint64_t m_ElapsedTime = 0;
		float m_Fps = 0;
	};
}
