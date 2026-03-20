#pragma once

/*
 Event system for handling user input and window events.
*/

namespace Core
{

	enum class Key
	{
		Unknown = 0,
		W, A, S, D,
		Escape,
		Space,
		Shift,
		RightCtrl,
		LeftCtrl,
		E,
		Q,
		T
	};

	enum class MouseButton
	{
		None = 0,
		Left,
		Right,
		Middle
	};

	enum class MouseMode
	{
		Normal,
		Captured
	};

	struct Event
	{
		enum class Type
		{
			None = 0,
			Quit,
			Keydown,
			Keyup,
			MouseMove,
			MouseButtonDown,
			MouseButtonUp,
			MouseWheelUp,
			MouseWheelDown,
			WindowResize
		} type;

		Key key;
		int mouseX;
		int mouseY;
		int mouseMotionX;
		int mouseMotionY;
		int wheelValue;
		MouseButton mouseButton;
	};
}
