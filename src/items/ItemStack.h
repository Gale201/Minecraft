#pragma once
#include <cstdint>

namespace Minecraft
{
	enum class ItemType
	{
		Undefined = 0,
		Grass,
		Dirt,
		Stone,
		OakPlanks,
		DoubleStoneSlab,
		PolishedStone,
		Brick,
		TNT,
		Cobblestone,
		Sand,
		Gravel,
		OakLog,
		OakLeaves,
		IronBlock,
		GoldBlock,
		GoldOre,
		IronOre,
		CoalOre,
		Bookshelf,
		Glass,
		Obsidian,

		Rose,
		YellowFlower
	};

	struct ItemStack
	{
		ItemType type;
		uint8_t size{ 0 };
	};
}
