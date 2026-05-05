#pragma once
#include <functional>

namespace Minecraft
{
	class ValueAnimation
	{
	private:
		static float LinearInterpolatorFunction(float start, float end, float t)
		{
			return start + (end - start) * t;
		}

		static float CubicEaseOutInterpolatorFunction(float start, float end, float t)
		{
			t = 1 - std::pow(1 - t, 3);
			return start + (end - start) * t;
		}

		static float SineEaseInInterpolatorFunction(float start, float end, float t)
		{
			t = 1 - std::cos((t * PI) / 2);
			return start + (end - start) * t;
		}

	public:
		inline static std::function<float(float, float, float)> s_LinearInterpolator = &ValueAnimation::LinearInterpolatorFunction;
		inline static std::function<float(float, float, float)> s_CubicEaseOutInterpolator = &ValueAnimation::CubicEaseOutInterpolatorFunction;
		inline static std::function<float(float, float, float)> s_SineEaseInInterpolator = &ValueAnimation::SineEaseInInterpolatorFunction;

	public:
		ValueAnimation(float startValue, float endValue, float duration, bool loop, bool loopReversed = false) :
			m_StartValue(startValue),
			m_EndValue(endValue),
			m_Duration(duration),
			m_Looping(loop),
			m_ReverseLooping(loopReversed)
		{
			m_Value = startValue;
			m_ElapsedTime = 0.0f;
			m_Finished = false;
		}

		void Update(float deltaTime)
		{
			if (m_Finished)
				return;

			m_ElapsedTime += deltaTime;
			if (m_ElapsedTime >= m_Duration)
			{
				m_ReverseDirection = !m_ReverseDirection;
				if (m_Looping)
					m_ElapsedTime = 0.0f;
				else
					m_Finished = true;
			}

			float t = m_ElapsedTime / m_Duration;
			if (m_ReverseLooping && m_ReverseDirection)
				t = 1.0f - t;
			m_Value = m_Interpolator(m_StartValue, m_EndValue, t);
		}

		void Finish() { m_Finished = true; m_Value = m_EndValue; }

		float GetValue() const { return m_Value; }

		void SetInterpolator(std::function<float(float, float, float)> interpolator)
		{
			m_Interpolator = interpolator;
		}

	private:
		float m_StartValue;
		float m_EndValue;
		float m_Duration;
		bool m_Looping;
		bool m_ReverseLooping;
		bool m_Finished;

		bool m_ReverseDirection = false;

		std::function<float(float, float, float)> m_Interpolator = s_LinearInterpolator;
	
		float m_ElapsedTime;
		float m_Value;
	};

}
