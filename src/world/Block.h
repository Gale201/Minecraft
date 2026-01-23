#pragma once

#include <cinttypes>

enum class BlockType : uint8_t
{
	Undefined = 0,
	Air,
	Grass,
	Dirt,
	Stone
};

inline const int BLOCK_TYPE_COUNT = 4;

struct Block
{
	BlockType type = BlockType::Air;
};
