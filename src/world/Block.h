#pragma once

#include <cinttypes>

namespace Minecraft
{
	enum class BlockType : uint8_t
	{
		Undefined = 0,
		Air,
		Grass,
		Dirt,
		Stone,
		Glass
	};

	inline const int BLOCK_TYPE_COUNT = 6;

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
				return false;
			default:
				return true;
			}
		}
	};
}
