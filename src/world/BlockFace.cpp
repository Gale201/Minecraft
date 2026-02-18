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
	case BlockType::Glass:
		return {
			{ {4,0}, {4,0}, {4,0}, {4,0}, {4,0}, {4,0} },
			{ 0.1f, 0.1f }
		};
	default:
		return {};
	}
}

AtlasTileInfo GetAtlasTileInfo(TileType type)
{
	switch (type)
	{
	case TileType::GrassSide:
		return {
			0.0f, 0.0f,
			0.1f, 0.1f
		};
	case TileType::Dirt:
		return {
			0.1f, 0.0f,
			0.1f, 0.1f
		};
	case TileType::GrassTop:
		return {
			0.2f, 0.0f,
			0.1f, 0.1f
		};
	case TileType::Stone:
		return {
			0.3f, 0.0f,
			0.1f, 0.1f
		};
	case TileType::Glass:
		return {
			0.4f, 0.0f,
			0.1f, 0.1f,
			true
		};
	default:
		return {};
	}
}

void GetFaceUV(BlockType type, FaceDirection dir, Vec2f& uvMin, Vec2f& uvMax, int uWraps, int vWraps)
{
	BlockTypeInfo info = GetBlockTypeInfo(type);
	Vec2f uvScale = info.uvScale;
	Vec2f uvOffset = info.uvOffset[(int)dir];
	uvOffset.x *= uvScale.x;
	uvOffset.y *= uvScale.y;

	float insetU = 0.01f / 160.f, insetV = 0.01f / 160.f;
	uvMin = { uvOffset.x + insetU, uvOffset.y + insetV };
	uvMax = { uvOffset.x + uvScale.x + uWraps - insetU, uvOffset.y + uvScale.y + vWraps - insetV };
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