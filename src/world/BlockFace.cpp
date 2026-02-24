#include "BlockFace.h"
#include "Block.h"

#include "../core/Log.h"

namespace Minecraft
{
	// Not needed anymore
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

	AtlasTileInfo CreateAtlasTileInfo(int x, int y, bool transparent)
	{
		return {
			x * ATLAS_TILE_SIZE / ATLAS_WIDTH,
			y * ATLAS_TILE_SIZE / ATLAS_HEIGHT,
			ATLAS_TILE_SIZE / ATLAS_WIDTH,
			ATLAS_TILE_SIZE / ATLAS_HEIGHT,
			static_cast<int>(ATLAS_WIDTH),
			static_cast<int>(ATLAS_HEIGHT),
			transparent
		};
	}

	AtlasTileInfo GetAtlasTileInfo(TileType type)
	{
		switch (type)
		{
		case TileType::GrassTop:
			return CreateAtlasTileInfo(0, 0);
		case TileType::Stone:
			return CreateAtlasTileInfo(1, 0);
		case TileType::Dirt:
			return CreateAtlasTileInfo(2, 0);
		case TileType::GrassSide:
			return CreateAtlasTileInfo(3, 0);
		case TileType::Glass:
			return CreateAtlasTileInfo(1, 3, true);
		case TileType::Water:
			return CreateAtlasTileInfo(14, 0, true);
		default:
			return {};
		}
	}

	TileType GetTileTypeFromDirection(BlockType type, FaceDirection dir)
	{
		TileType tileType = TileType::Undefined;
		if (type == BlockType::Grass)
		{
			if (dir == FaceDirection::PosY) tileType = TileType::GrassTop;
			else if (dir == FaceDirection::NegY) tileType = TileType::Dirt;
			else tileType = TileType::GrassSide;
		}
		else if (type == BlockType::Dirt) tileType = TileType::Dirt;
		else if (type == BlockType::Stone) tileType = TileType::Stone;
		else if (type == BlockType::Glass) tileType = TileType::Glass;
		else if (type == BlockType::Water) tileType = TileType::Water;
		return tileType;
	}

	BlockFace GetBlockFace(BlockType type, FaceDirection dir, bool isTop)
	{
		if (type == BlockType::Water && isTop)
		{
			switch (dir)
			{
			case FaceDirection::PosX:
				return {
					{ {1,0,0}, {1.0f,0.9f,1.0f}, {1.0f,0.9f,1.0f}, {1,0,0} },
					{1,0,0},
					{ {1.0f, 0.9f}, {1, 0}, {0, 0}, {0.0f, 1.0f} }
				};
			case FaceDirection::NegX:
				return {
					{ {0,0,1}, {0.0f,0.9f,1.0f}, {0.0f,0.9f,0.0f}, {0,0,0} },
					{-1,0,0},
					{ {1.0f,0.9f},{1,0},{0,0},{0.0f,0.9f} }
				};
			case FaceDirection::PosY:
				return {
					{ {0.f,0.9f,0.f}, {0.f,0.9f,1.f}, {1.f,0.9f,1.f}, {1.f,0.9f,0.f} }, 
					{0,1,0},
					{ {1,1},{1,0},{0,0},{0,1} } 
				};
			case FaceDirection::NegY:
				return { 
					{ {0,0,1}, {0,0,0}, {1,0,0}, {1,0,1} }, 
					{0,-1,0}, 
					{ {1,1},{1,0},{0,0},{0,1} }
				};
			case FaceDirection::PosZ:
				return { 
					{ {1,0,1}, {1.f,0.9f,1.f}, {0.f,0.9f,1.f}, {0,0,1} }, 
					{0,0,1}, 
					{ {1.f,0.9f},{1,0},{0,0},{0.f,0.9f} }
				};
			case FaceDirection::NegZ:
				return {
					{ {0,0,0}, {0.f,0.9f,0.f}, {1.f,0.9f,0.f}, {1,0,0} },
					{0,0,-1},
					{ {1.f,0.9f},{1,0},{0,0},{0.f,0.9f} }
				};
			}
		}
		return CUBE_FACES[static_cast<int>(dir)];
	}

	// This function is broken currently
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
}
