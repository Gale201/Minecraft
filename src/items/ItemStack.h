#pragma once
#include <cstdint>

namespace Minecraft
{
	enum class ItemType
	{
		Grass,
		Dirt,
		Stone,
		Glass,
		Rose,
		Undefined
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

		static bool IsItemBlock(ItemType type)
		{
			switch (type)
			{
			case ItemType::Grass:
			case ItemType::Dirt:
			case ItemType::Stone:
			case ItemType::Glass:
				return true;
			default:
				return false;
			}
		}
	};
}
