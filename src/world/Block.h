#pragma once

#include <cinttypes>

#include "..\math\Math.h"

namespace Minecraft
{
	enum class BlockType : uint8_t
	{
		Undefined = 0,
		Air,
		Grass,
		Dirt,
		Stone,
		OakPlanks,
		DoubleStoneSlab,
		PolishedStone,
		Brick,
		TNT,
		Cobweb,
		Rose,
		YellowFlower,
		Water,
		OakSapling,
		Cobblestone,
		Bedrock,
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
		Obsidian
	};

	struct Block
	{
		BlockType type = BlockType::Air;

		bool IsTransparent() const
		{
			switch (type)
			{
			case BlockType::Undefined:
			case BlockType::Air:
			case BlockType::Glass:
			case BlockType::Water:
				return true;
			default:
				return false;
			}
		}

		bool IsCutoffTransparent() const
		{
			switch (type)
			{
			case BlockType::Cobweb:
			case BlockType::Rose:
			case BlockType::YellowFlower:
			case BlockType::OakSapling:
			case BlockType::OakLeaves:
				return true;
			default:
				return false;
			}
		}

		bool IsCollidable() const
		{
			switch (type)
			{
			case BlockType::Undefined:
			case BlockType::Air:
			case BlockType::Water:
			case BlockType::Cobweb:
			case BlockType::Rose:
			case BlockType::YellowFlower:
			case BlockType::OakSapling:
				return false;
			default:
				return true;
			}
		}

		bool isCollidableWithRaycasts() const
		{
			switch (type)
			{
			case BlockType::Undefined:
			case BlockType::Air:
			case BlockType::Water:
				return false;
			default:
				return true;
			}
		}

		bool IsFullBlock() const
		{
			switch (type)
			{
			case BlockType::Rose:
			case BlockType::YellowFlower:
			case BlockType::OakSapling:
				return false;
			default:
				return true;
			}
		}

		bool IsBreakable() const
		{
			switch (type)
			{
			case BlockType::Air:
				return false;
			case BlockType::Water:
				return false;
			default:
				return true;
			}
		}

		float GetBreakTime() const
		{
			float level1 = 1.0f;
			float level2 = 2.0f;
			float level3 = 6.0f;

			switch (type)
			{
			case BlockType::Grass: return level1;
			case BlockType::Dirt: return level1;
			case BlockType::Stone: return level3;
			case BlockType::OakPlanks: return level2;
			case BlockType::DoubleStoneSlab: return level3;
			case BlockType::PolishedStone: return level3;
			case BlockType::Brick: return level3;
			case BlockType::TNT: return 0.0f;
			case BlockType::Cobweb: return 4.0f;
			case BlockType::Rose: return 0.0f;
			case BlockType::YellowFlower: return 0.0f;
			case BlockType::Water: return -1.0f;
			case BlockType::OakSapling: return 0.0f;
			case BlockType::Cobblestone: return level3;
			case BlockType::Bedrock: return -1.0f;
			case BlockType::Sand: return level1;
			case BlockType::Gravel: return level1;
			case BlockType::OakLog: return level2;
			case BlockType::OakLeaves: return 0.2f;
			case BlockType::IronBlock: return level3;
			case BlockType::GoldBlock: return level3;
			case BlockType::GoldOre: return level3;
			case BlockType::IronOre: return level3;
			case BlockType::CoalOre: return level3;
			case BlockType::Bookshelf: return level3;
			case BlockType::Glass: return 0.3f;
			case BlockType::Obsidian: return 50.0f;
			default:
				return 0.0f;
			}
		}
	};
}
