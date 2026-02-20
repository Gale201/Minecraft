#include "InputManager.h"

namespace Core
{
	void InputManager::ProcessEvents(const std::vector<Event>& events)
	{
		m_MouseMotionX = 0;
		m_MouseMotionY = 0;
		m_KeysJustPressed.clear();
		m_KeysJustReleased.clear();
		m_MouseButtonsJustPressed.clear();
		m_MouseButtonsJustReleased.clear();
		for (const Event& e : events)
		{
			if (e.type == Event::Type::Keydown)
			{
				if (!m_KeysDown[e.key])
				{
					m_KeysJustPressed[e.key] = true;
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
}
