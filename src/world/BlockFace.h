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
		NegZ,

		PosXPosZ,
		PosXNegZ,
		NegXPosZ,
		NegXNegZ
	};

	struct BlockFace
	{
		Vec3f vertices[4];
		Vec3f normal;
		Vec2f uvs[4];
	};

	struct BlockBoundingBox
	{
		Vec3f offset;
		Vec3f size;
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
		OakPlank,
		DoubleStoneSlab,
		PolishedStone,
		Brick,
		TNTSide,
		TNTTop,
		TNTBottom,
		Cobweb,
		Rose,
		YellowFlower,
		Water,
		OakSapling,
		Cobblestone,
		Bedrock,
		Sand,
		Gravel,
		OakLogSide,
		OakLogTop,
		OakLeaves,
		IronBlock,
		GoldBlock,
		GoldOre,
		IronOre,
		CoalOre,
		Bookshelf,
		Glass
	};

	struct AtlasTileInfo
	{
		float u0, v0;
		float uScale, vScale;
		int atlasWidth;
		int atlasHeight;
		bool isTransparent = false;
		bool isCutout = false;
	};

	AtlasTileInfo CreateAtlasTileInfo(int x, int y, bool transparent = false, bool cutout = false);

	void GetFaceUV(BlockType type, FaceDirection dir, Vec2f& uvScale, Vec2f& uvOffset, int uWraps, int vWraps);

	AtlasTileInfo GetAtlasTileInfo(TileType type);

	TileType GetTileTypeFromDirection(BlockType type, FaceDirection dir);

	BlockFace GetBlockFace(BlockType type, FaceDirection dir, bool isTop = false);

	BlockBoundingBox GetBlockBoundingBox(BlockType type);

	Vec3i GetCubeNeighborOffset(FaceDirection dir);

	extern const FaceDirection FACE_DIRECTIONS[6];
	extern const FaceDirection X_FACE_DIRECTIONS[4];
}
