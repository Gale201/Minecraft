#pragma once

/*
 Event system for handling user input and window events.
*/

enum class Key
{
	Unknown = 0,
	W, A, S, D,
	Escape,
	Space,
	Shift
};

enum class MouseButton
{
	None = 0,
	Left,
	Right,
	Middle
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
		MouseButtonUp
	} type;

	Key key;
	int mouseX;
	int mouseY;
	int mouseMotionX;
	int mouseMotionY;
	MouseButton mouseButton;
};
