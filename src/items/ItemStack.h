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

		Rose,
		YellowFlower
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
			case ItemType::OakPlanks:
			case ItemType::DoubleStoneSlab:
			case ItemType::PolishedStone:
			case ItemType::Brick:
			case ItemType::TNT:
			case ItemType::Cobblestone:
			case ItemType::Sand:
			case ItemType::Gravel:
			case ItemType::OakLog:
			case ItemType::OakLeaves:
			case ItemType::IronBlock:
			case ItemType::GoldOre:
			case ItemType::IronOre:
			case ItemType::CoalOre:
			case ItemType::Bookshelf:
			case ItemType::Glass:
				return true;
			default:
				return false;
			}
		}

		static bool IsItemPlacable(ItemType type)
		{
			if (IsItemBlock(type)) return true;

			switch (type)
			{
			case ItemType::Rose:
			case ItemType::YellowFlower:
				return true;
			default:
				return false;
			}
		}
	};
}
