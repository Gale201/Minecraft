#pragma once

#include <cinttypes>

#include "Block.h"
#include "../math/Math.h"

#define ATLAS_WIDTH 256.0f
#define ATLAS_HEIGHT 256.0f
#define ATLAS_TILE_SIZE 16.0f

namespace Minecraft
{
	enum class FaceDirection : int
	{
		PosX = 0,
		NegX,
		PosY,
		NegY,
		PosZ,
		NegZ
	};

	struct BlockFace
	{
		Vec3f vertices[4];
		Vec3f normal;
		Vec2f uvs[4];
	};

	// Not needed anymore
	struct BlockTypeInfo
	{
		Vec2f uvOffset[6];
		Vec2f uvScale;
	};

	enum class TileType : uint8_t
	{
		Undefined = 0,
		GrassSide,
		Dirt,
		GrassTop,
		Stone,
		Glass,
		Water
	};

	struct AtlasTileInfo
	{
		float u0, v0;
		float uScale, vScale;
		int atlasWidth;
		int atlasHeight;
		bool isTransparent = false;
	};

	AtlasTileInfo CreateAtlasTileInfo(int x, int y, bool transparent = false);

	void GetFaceUV(BlockType type, FaceDirection dir, Vec2f& uvScale, Vec2f& uvOffset, int uWraps, int vWraps);

	AtlasTileInfo GetAtlasTileInfo(TileType type);

	TileType GetTileTypeFromDirection(BlockType type, FaceDirection dir);

	BlockFace GetBlockFace(BlockType type, FaceDirection dir, bool isTop = false);

	extern const FaceDirection FACE_DIRECTIONS[6];

	extern const BlockFace CUBE_FACES[6];

	extern const Vec3i CUBE_NEIGHBOR_OFFSET[6];
}
