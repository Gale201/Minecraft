#pragma once

/*
 Event system for handling user input and window events.
*/

enum class Key
{
	UNKNOWN = 0,
	W, A, S, D,
	ESCAPE,
	SPACE,
	SHIFT
};

enum class MouseButton
{
	NONE = 0,
	LEFT,
	RIGHT,
	MIDDLE
};

struct Event
{
	enum class Type
	{
		NONE = 0,
		QUIT,
		KEYDOWN,
		KEYUP,
		MOUSEMOVE,
		MOUSEBUTTONDOWN,
		MOUSEBUTTONUP
	} type;

	Key key;
	int mouseX;
	int mouseY;
	int mouseMotionX;
	int mouseMotionY;
	MouseButton mouseButton;
};
