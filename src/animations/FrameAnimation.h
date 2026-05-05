#pragma once

namespace Minecraft
{
	class FrameAnimation
	{
	public:
		FrameAnimation(int frameCount, float frameDuration) :
			m_FrameCount(frameCount), m_FrameDuration(frameDuration), 
			m_CurrentFrame(0), m_ElapsedTime(0.0f)
		{
		}

		void Update(float deltaTime)
		{
			m_JustFinishedCycle = false;

			m_ElapsedTime += deltaTime;
			while (m_ElapsedTime >= m_FrameDuration)
			{
				m_ElapsedTime -= m_FrameDuration;
				m_CurrentFrame = (m_CurrentFrame + 1) % m_FrameCount;
				if (m_CurrentFrame == m_FrameCount - 1)
					m_JustFinishedCycle = true;
			}
		}

		void Reset()
		{
			m_CurrentFrame = 0;
			m_ElapsedTime = 0.0f;
			m_JustFinishedCycle = false;
		}

		bool JustFinishedCycle() const { return m_JustFinishedCycle; }
		int GetCurrentFrame() const { return m_CurrentFrame; }

	private:
		int m_FrameCount;
		float m_FrameDuration;
		int m_CurrentFrame;
		float m_ElapsedTime;

		bool m_JustFinishedCycle = false;
	};
}