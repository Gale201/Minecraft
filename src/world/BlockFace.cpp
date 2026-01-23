#include "BlockFace.h"
#include "Block.h"
#include "../core/Log.h"

BlockTypeInfo GetBlockTypeInfo(BlockType type)
{
	switch (type)
	{
	case BlockType::Grass:
		return {
			{ {0,0}, {0,0}, {2,0}, {1,0}, {0,0}, {0,0} },
			{ 0.1f, 0.1f }
		};
	case BlockType::Dirt:
		return {
			{ {1,0}, {1,0}, {1,0}, {1,0}, {1,0}, {1,0} },
			{ 0.1f, 0.1f }
		};
	case BlockType::Stone:
		return {
			{ {3,0}, {3,0}, {3,0}, {3,0}, {3,0}, {3,0} },
			{ 0.1f, 0.1f }
		};
	default:
		return {};
	}
}

void GetFaceUV(BlockType type, FaceDirection dir, Vec2f& uvScale, Vec2f& uvOffset)
{
	BlockTypeInfo info = GetBlockTypeInfo(type);
	uvScale = info.uvScale;
	uvOffset = info.uvOffset[(int)dir];
}

const FaceDirection FACE_DIRECTIONS[6] = {
	FaceDirection::PosX,
	FaceDirection::NegX,
	FaceDirection::PosY,
	FaceDirection::NegY,
	FaceDirection::PosZ,
	FaceDirection::NegZ
};

const BlockFace CUBE_FACES[6] = {
	// PosX
	{ { {1,0,0}, {1,1,0}, {1,1,1}, {1,0,1} }, {1,0,0}, { {1,1},{1,0},{0,0},{0,1} } },
	// NegX
	{ { {0,0,1}, {0,1,1}, {0,1,0}, {0,0,0} }, {-1,0,0}, { {1,1},{1,0},{0,0},{0,1} } },
	// PosY
	{ { {0,1,0}, {0,1,1}, {1,1,1}, {1,1,0} }, {0,1,0}, { {1,1},{1,0},{0,0},{0,1} } },
	// NegY
	{ { {0,0,1}, {0,0,0}, {1,0,0}, {1,0,1} }, {0,-1,0}, { {1,1},{1,0},{0,0},{0,1} } },
	// PosZ
	{ { {1,0,1}, {1,1,1}, {0,1,1}, {0,0,1} }, {0,0,1}, { {1,1},{1,0},{0,0},{0,1} } },
	// NegZ
	{ { {0,0,0}, {0,1,0}, {1,1,0}, {1,0,0} }, {0,0,-1}, { {1,1},{1,0},{0,0},{0,1} } }
};

const Vec3i CUBE_NEIGHBOR_OFFSET[6] = {
	{  1, 0, 0 },
	{ -1, 0, 0 },
	{ 0,  1, 0 },
	{ 0, -1, 0 },
	{ 0, 0,  1 },
	{ 0, 0, -1 }
};