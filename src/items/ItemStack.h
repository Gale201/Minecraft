#pragma once
#include <cstdint>

namespace Minecraft
{
	enum class ItemType
	{
		Grass,
		Dirt,
		Stone,
		Glass
	};

	struct ItemStack
	{
		ItemType type;
		uint8_t size{ 0 };

		static uint8_t GetSizeForType(ItemType type)
		{
			switch (type)
			{
			default: return 64;
			};
		}
	};
}
