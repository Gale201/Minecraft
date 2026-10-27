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
		T,
		K_0, K_1, K_2, K_3, K_4, K_5, K_6, K_7, K_8, K_9,
		F3,
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
