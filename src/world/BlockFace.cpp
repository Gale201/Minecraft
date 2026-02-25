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
		case TileType::OakPlank:
			return CreateAtlasTileInfo(4, 0);
		case TileType::DoubleStoneSlab:
			return CreateAtlasTileInfo(5, 0);
		case TileType::PolishedStone:
			return CreateAtlasTileInfo(6, 0);
		case TileType::Brick:
			return CreateAtlasTileInfo(7, 0);
		case TileType::TNTSide:
			return CreateAtlasTileInfo(8, 0);
		case TileType::TNTTop:
			return CreateAtlasTileInfo(9, 0);
		case TileType::TNTBottom:
			return CreateAtlasTileInfo(10, 0);
		case TileType::Cobweb:
			return CreateAtlasTileInfo(11, 0, true);
		case TileType::Rose:
			return CreateAtlasTileInfo(12, 0, true);
		case TileType::YellowFlower:
			return CreateAtlasTileInfo(13, 0, true);
		case TileType::Water:
			return CreateAtlasTileInfo(14, 0, true);
		case TileType::OakSapling:
			return CreateAtlasTileInfo(15, 0);
		case TileType::Cobblestone:
			return CreateAtlasTileInfo(0, 1);
		case TileType::Bedrock:
			return CreateAtlasTileInfo(1, 1);
		case TileType::Sand:
			return CreateAtlasTileInfo(2, 1);
		case TileType::Gravel:
			return CreateAtlasTileInfo(3, 1);
		case TileType::OakLogSide:
			return CreateAtlasTileInfo(4, 1);
		case TileType::OakLogTop:
			return CreateAtlasTileInfo(5, 1);
		case TileType::OakLeaves:
			return CreateAtlasTileInfo(6, 1, true);
		case TileType::IronBlock:
			return CreateAtlasTileInfo(7, 1);
		case TileType::GoldBlock:
			return CreateAtlasTileInfo(8, 1);
		case TileType::GoldOre:
			return CreateAtlasTileInfo(0, 2);
		case TileType::IronOre:
			return CreateAtlasTileInfo(1, 2);
		case TileType::CoalOre:
			return CreateAtlasTileInfo(2, 2);
		case TileType::Bookshelf:
			return CreateAtlasTileInfo(3, 2);
		case TileType::Glass:
			return CreateAtlasTileInfo(1, 3, true);
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
		else if (type == BlockType::DoubleStoneSlab) tileType = TileType::DoubleStoneSlab;
		else if (type == BlockType::PolishedStone) tileType = TileType::PolishedStone;
		else if (type == BlockType::Brick) tileType = TileType::Brick;
		else if (type == BlockType::TNT)
		{
			if (dir == FaceDirection::PosY) tileType = TileType::TNTTop;
			else if (dir == FaceDirection::NegY) tileType = TileType::TNTBottom;
			else tileType = TileType::TNTSide;
		}
		else if (type == BlockType::Cobweb) tileType = TileType::Cobweb;
		else if (type == BlockType::Rose) tileType = TileType::Rose;
		else if (type == BlockType::YellowFlower) tileType = TileType::YellowFlower;
		else if (type == BlockType::Water) tileType = TileType::Water;
		else if (type == BlockType::OakSapling) tileType = TileType::OakSapling;
		else if (type == BlockType::Cobblestone) tileType = TileType::Cobblestone;
		else if (type == BlockType::Bedrock) tileType = TileType::Bedrock;
		else if (type == BlockType::Sand) tileType = TileType::Sand;
		else if (type == BlockType::Gravel) tileType = TileType::Gravel;
		else if (type == BlockType::OakLog)
		{
			if (dir == FaceDirection::PosY || dir == FaceDirection::NegY) tileType = TileType::OakLogTop;
			else tileType = TileType::OakLogSide;
		}
		else if (type == BlockType::OakLeaves) tileType = TileType::OakLeaves;
		else if (type == BlockType::IronBlock) tileType = TileType::IronBlock;
		else if (type == BlockType::GoldBlock) tileType = TileType::GoldBlock;
		else if (type == BlockType::GoldOre) tileType = TileType::GoldOre;
		else if (type == BlockType::IronOre) tileType = TileType::IronOre;
		else if (type == BlockType::CoalOre) tileType = TileType::CoalOre;
		else if (type == BlockType::Bookshelf) tileType = TileType::Bookshelf;
		else if (type == BlockType::Glass) tileType = TileType::Glass;
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
