#include "BlockFace.h"
#include "Block.h"

#include "../core/Log.h"

namespace Minecraft
{
	const FaceDirection FACE_DIRECTIONS[6] = {
		FaceDirection::PosX,
		FaceDirection::NegX,
		FaceDirection::PosY,
		FaceDirection::NegY,
		FaceDirection::PosZ,
		FaceDirection::NegZ
	};

	const FaceDirection X_FACE_DIRECTIONS[4] = {
		FaceDirection::PosXPosZ,
		FaceDirection::PosXNegZ,
		FaceDirection::NegXPosZ,
		FaceDirection::NegXNegZ
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

	const BlockFace X_FACES[4] = {
		// PosXPosZ
		{ { {0,0,0}, {0,1,0}, {1,1,1}, {1,0,1} }, {1, 0, 1}, { {1,1}, {1,0}, {0,0}, {0,1}}},
		// PosXNegZ
		{ { {0,0,1}, {0,1,1}, {1,1,0}, {1,0,0} }, {1, 0, -1}, { {1,1}, {1,0}, {0,0}, {0,1}}},
		// NegXPosZ
		{ { {1,0,0}, {1,1,0}, {0,1,1}, {0,0,1} }, {-1, 0, 1}, { {1,1}, {1,0}, {0,0}, {0,1}}},
		// NegXNegZ
		{ { {1,0,1}, {1,1,1}, {0,1,0}, {0,0,0} }, {-1, 0, -1}, { {1,1}, {1,0}, {0,0}, {0,1}}},
	};

	const Vec3i CUBE_NEIGHBOR_OFFSET[6] = {
		{  1, 0, 0 },
		{ -1, 0, 0 },
		{ 0,  1, 0 },
		{ 0, -1, 0 },
		{ 0, 0,  1 },
		{ 0, 0, -1 }
	};

	AtlasTileInfo CreateAtlasTileInfo(
		int x, int y, bool transparent, bool cutout, 
		float atlasWidth, float atlasHeight, float atlasTileSize
	)
	{
		return {
			x * atlasTileSize / atlasWidth,
			y * atlasTileSize / atlasHeight,
			atlasTileSize / atlasWidth,
			atlasTileSize / atlasHeight,
			static_cast<int>(atlasWidth),
			static_cast<int>(atlasHeight),
			transparent,
			cutout
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
			return CreateAtlasTileInfo(11, 0, false, true);
		case TileType::Rose:
			return CreateAtlasTileInfo(12, 0, false, true);
		case TileType::YellowFlower:
			return CreateAtlasTileInfo(13, 0, false, true);
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
			return CreateAtlasTileInfo(6, 1, false, true);
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
			return CreateAtlasTileInfo(1, 3, false, true);
		case TileType::Obsidian:
			return CreateAtlasTileInfo(5, 2);
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
		else if (type == BlockType::Obsidian) tileType = TileType::Obsidian;

		return tileType;
	}

	BlockFace GetBlockFace(BlockType type, FaceDirection dir, bool isTop)
	{
		size_t direction = static_cast<size_t>(dir);

		if (type == BlockType::Water && isTop)
		{
			switch (dir)
			{
			case FaceDirection::PosX:
				return {
					{ {1,0,0}, {1.0f,0.9f,0.0f}, {1.0f,0.9f,1.0f}, {1,0,1} },
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
		if (direction < 6)
			return CUBE_FACES[direction];
		else if (direction < 10)
			return X_FACES[direction - 6];

		return {};
	}

	BlockBoundingBox GetBlockBoundingBox(BlockType type)
	{
		switch (type)
		{
		case BlockType::Rose:
		case BlockType::YellowFlower:
			return { { 0.3f, 0.0f, 0.3f }, { 0.4f, 0.6f, 0.4f } };
		case BlockType::OakSapling:
			return {};
		case BlockType::Water:
			return { { 0.0f, 0.0f, 0.0f }, { 1.0f, 0.9f, 1.0f } };
		default:
			return { { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f } };
		}
	}

	Vec3i GetCubeNeighborOffset(FaceDirection dir)
	{
		if (static_cast<size_t>(dir) < 6)
			return CUBE_NEIGHBOR_OFFSET[static_cast<size_t>(dir)];
		return Vec3i();
	}
}
