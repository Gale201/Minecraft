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
		OakPlank,
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
		Glass
	};

	inline const int BLOCK_TYPE_COUNT = 7;

	struct Block
	{
		BlockType type = BlockType::Air;

		bool IsTransparent()
		{
			switch (type)
			{
			case BlockType::Undefined:
			case BlockType::Air:
			case BlockType::Glass:
			case BlockType::Water:
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

		bool IsCollidable()
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

		bool isCollidableWithRaycasts()
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
	};
}
