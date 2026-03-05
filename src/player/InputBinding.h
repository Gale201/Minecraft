#pragma once

#include <unordered_map>

#include "../core/Event.h"

using Core::Key, Core::MouseButton;

namespace Minecraft
{
	enum class InputAction
	{
		MoveForward,
		MoveBackward,
		MoveLeft,
		MoveRight,
		MoveUp,
		MoveDown,
		Look,
		Jump,
		Sprint,
		Sneak,
		BreakBlock,
		PlaceBlock,
		DropItem,
		OpenInventory,
		OpenChat
	};

	struct InputBindings
	{
		std::unordered_map<InputAction, Key> keyBindings;
		std::unordered_map<InputAction, MouseButton> mouseBindings;

		void AddKeyBinding(InputAction ia, Key key)
		{
			keyBindings.insert({ ia, key });
		}

		void AddMouseButtonBinding(InputAction ia, MouseButton button)
		{
			mouseBindings.insert({ ia, button });
		}

		Key GetKeyForAction(InputAction ia) const
		{
			if (keyBindings.find(ia) == keyBindings.end())
				return Key::Unknown;
			return keyBindings.at(ia);
		}

		MouseButton GetMouseButtonForAction(InputAction ia) const
		{
			if (mouseBindings.find(ia) == mouseBindings.end())
				return MouseButton::None;
			return mouseBindings.at(ia);
		}
	};
}
