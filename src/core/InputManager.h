#pragma once

#include <unordered_map>

#include "Event.h"

/*
 InputManager handles keyboard and mouse input states.
 It tracks which keys and mouse buttons are currently pressed,
 which were just pressed or released in the current frame
 and the current mouse position.
*/

namespace Core
{
	class InputManager
	{
	public:
		void ProcessEvents(const std::vector<Event>& events);

		bool IsKeyDown(Key key) const;
		bool IsKeyJustPressed(Key key) const;
		bool IsKeyJustReleased(Key key) const;

		bool IsMouseButtonDown(MouseButton button) const;
		bool IsMouseButtonJustPressed(MouseButton button) const;
		bool IsMouseButtonJustReleased(MouseButton button) const;

		int GetMouseX() const { return m_MouseX; }
		int GetMouseY() const { return m_MouseY; }
		int GetMouseMotionX() const { return m_MouseMotionX; }
		int GetMouseMotionY() const { return m_MouseMotionY; }

	private:
		std::unordered_map<Key, bool> m_KeysDown;
		std::unordered_map<Key, bool> m_KeysJustPressed;
		std::unordered_map<Key, bool> m_KeysJustReleased;

		std::unordered_map<MouseButton, bool> m_MouseButtonsJustPressed;
		std::unordered_map<MouseButton, bool> m_MouseButtonsJustReleased;
		std::unordered_map<MouseButton, bool> m_MouseButtonsDown;
		int m_MouseX = 0;
		int m_MouseY = 0;
		int m_MouseMotionX = 0;
		int m_MouseMotionY = 0;
	};
}
