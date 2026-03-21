#include "InputManager.h"

#include <chrono>

namespace Core
{
	void InputManager::ProcessEvents(const std::vector<Event>& events)
	{
		m_MouseMotionX = 0;
		m_MouseMotionY = 0;
		m_MouseWheelMotion = 0;
		m_KeysJustPressed.clear();
		m_KeysJustReleased.clear();
		m_KeysJustDoublePressed.clear();
		m_MouseButtonsJustPressed.clear();
		m_MouseButtonsJustReleased.clear();
		m_MouseButtonsJustDoublePressed.clear();

		m_JustResized = false;

		for (const Event& e : events)
		{
			if (e.type == Event::Type::Keydown)
			{
				if (!m_KeysDown[e.key])
				{
					m_KeysJustPressed[e.key] = true;
					auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
					auto miliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
					if (miliseconds - m_KeyPressTimeTable[e.key] < m_DoublePressMaxDelay)
					{
						m_KeysJustDoublePressed[e.key] = true;
					}
					m_KeyPressTimeTable[e.key] = miliseconds;
				}
				m_KeysDown[e.key] = true;
			}
			else if (e.type == Event::Type::Keyup)
			{
				if (m_KeysDown[e.key])
				{
					m_KeysJustReleased[e.key] = true;
				}
				m_KeysDown[e.key] = false;
			}
			else if (e.type == Event::Type::MouseButtonDown)
			{
				if (!m_MouseButtonsDown[e.mouseButton])
				{
					m_MouseButtonsJustPressed[e.mouseButton] = true;
					auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
					auto miliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
					if (miliseconds - m_MouseButtonPressTimeTable[e.mouseButton] < m_DoublePressMaxDelay)
					{
						m_MouseButtonsJustDoublePressed[e.mouseButton] = true;
					}
					m_MouseButtonPressTimeTable[e.mouseButton] = miliseconds;
				}
				m_MouseButtonsDown[e.mouseButton] = true;
			}
			else if (e.type == Event::Type::MouseButtonUp)
			{
				if (m_MouseButtonsDown[e.mouseButton])
				{
					m_MouseButtonsJustReleased[e.mouseButton] = true;
				}
				m_MouseButtonsDown[e.mouseButton] = false;
			}
			else if (e.type == Event::Type::MouseMove)
			{
				m_MouseX = e.mouseX;
				m_MouseY = e.mouseY;
				m_MouseMotionX += e.mouseMotionX;
				m_MouseMotionY += e.mouseMotionY;
			}
			else if (e.type == Event::Type::MouseWheelDown)
				m_MouseWheelMotion += e.wheelValue;
			else if (e.type == Event::Type::MouseWheelUp)
				m_MouseWheelMotion += e.wheelValue;
			else if (e.type == Event::Type::WindowResize)
				m_JustResized = true;
		}
	}

	bool InputManager::IsKeyDown(Key key) const
	{
		auto it = m_KeysDown.find(key);
		if (it != m_KeysDown.end())
		{
			return it->second;
		}
		return false;
	}

	bool InputManager::IsKeyJustPressed(Key key) const
	{
		auto it = m_KeysJustPressed.find(key);
		if (it != m_KeysJustPressed.end())
		{
			return it->second;
		}
		return false;
	}

	bool InputManager::IsKeyJustReleased(Key key) const
	{
		auto it = m_KeysJustReleased.find(key);
		if (it != m_KeysJustReleased.end())
		{
			return it->second;
		}
		return false;
	}

	bool InputManager::IsKeyJustDoublePressed(Key key) const
	{
		auto it = m_KeysJustDoublePressed.find(key);
		if (it != m_KeysJustDoublePressed.end())
			return it->second;
		return false;
	}

	bool InputManager::IsMouseButtonDown(MouseButton button) const
	{
		auto it = m_MouseButtonsDown.find(button);
		if (it != m_MouseButtonsDown.end())
		{
			return it->second;
		}
		return false;
	}

	bool InputManager::IsMouseButtonJustPressed(MouseButton button) const
	{
		auto it = m_MouseButtonsJustPressed.find(button);
		if (it != m_MouseButtonsJustPressed.end())
		{
			return it->second;
		}
		return false;
	}

	bool InputManager::IsMouseButtonJustReleased(MouseButton button) const
	{
		auto it = m_MouseButtonsJustReleased.find(button);
		if (it != m_MouseButtonsJustReleased.end())
		{
			return it->second;
		}
		return false;
	}

	bool InputManager::IsMouseButtonJustDoublePressed(MouseButton button) const
	{
		auto it = m_MouseButtonsJustDoublePressed.find(button);
		if (it != m_MouseButtonsJustDoublePressed.end())
			return it->second;
		return false;
	}

	bool InputManager::ResizeJustHappened() const
	{
		return m_JustResized;
	}
}
