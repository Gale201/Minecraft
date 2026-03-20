#include "ItemRegistry.h"
#include "Item.h"

namespace Minecraft
{
	void ItemRegistry::Initialize()
	{
		for (ItemType type : m_ItemTypes)
		{
			if (IsItemFullBlock(type))
			{
				BlockType blockType = GetBlockTypeFromItemType(type);

				Mesh mesh;
				for (int dir = 0; dir < 6; dir++)
				{
					int startIndex = mesh.vertices.size();

					FaceDirection faceDir = static_cast<FaceDirection>(dir);
					BlockFace face = GetBlockFace(blockType, faceDir);
					
					for (int i = 0; i < 4; i++)
					{
						Vertex v;
						v.position = Vec3f(
							face.vertices[i].x - 0.5f,
							face.vertices[i].y - 0.5f,
							face.vertices[i].z - 0.5f
						) * m_BlockItemScaleFactor;
						v.normal = face.normal;

						v.u = face.uvs[i].x;
						v.v = face.uvs[i].y;

						TileType tileType = GetTileTypeFromDirection(blockType, faceDir);
						v.tileInfo = GetAtlasTileInfo(tileType);

						mesh.vertices.push_back(v);
					}

					mesh.indices.push_back(startIndex + 0);
					mesh.indices.push_back(startIndex + 1);
					mesh.indices.push_back(startIndex + 2);

					mesh.indices.push_back(startIndex + 0);
					mesh.indices.push_back(startIndex + 2);
					mesh.indices.push_back(startIndex + 3);
				}
				m_Meshes.emplace(type, std::move(mesh));
			}
			else
			{			
				Mesh mesh;
				AtlasTileInfo tileInfo = GetAtlasTileInfoForItem(type);

				float yScale = 0.12f;

				FaceDirection dirs[2] = { FaceDirection::PosY, FaceDirection::NegY };
				for (int dir = 0; dir < 2; dir++)
				{
					int startIndex = static_cast<int>(mesh.vertices.size());
					BlockFace face = GetBlockFace(BlockType::Dirt, dirs[dir]);
					for (int i = 0; i < 4; i++)
					{
						Vertex v;
						v.position = Vec3f(
							face.vertices[i].x - 0.5f,
							face.vertices[i].y,
							face.vertices[i].z - 0.5f
						) * m_ItemScaleFactor;
						v.position.y *= yScale;
						v.position.y -= m_ItemHitboxSize.y / 2.f;
						v.normal = face.normal;

						v.u = face.uvs[i].x;
						v.v = face.uvs[i].y;
						if (dir == 1) v.v = 1.0f - v.v;

						v.tileInfo = tileInfo;

						mesh.vertices.push_back(v);
					}

					mesh.indices.push_back(startIndex + 0);
					mesh.indices.push_back(startIndex + 1);
					mesh.indices.push_back(startIndex + 2);

					mesh.indices.push_back(startIndex + 0);
					mesh.indices.push_back(startIndex + 2);
					mesh.indices.push_back(startIndex + 3);
				}
				
				int u = static_cast<int>(tileInfo.u0 * tileInfo.atlasWidth);
				int v = static_cast<int>(tileInfo.v0 * tileInfo.atlasHeight);
				int w = static_cast<int>(tileInfo.uScale * tileInfo.atlasWidth);
				int h = static_cast<int>(tileInfo.vScale * tileInfo.atlasHeight);
				Texture tileTexture = m_TextureLoader.Get("item-atlas").Crop(u, v, w, h);

				AtlasTileInfo sideTileInfo;
				sideTileInfo.isCutout = true;
				sideTileInfo.uScale = tileInfo.uScale / (float)tileTexture.GetWidth();
				sideTileInfo.vScale = tileInfo.vScale / (float)tileTexture.GetHeight();
				sideTileInfo.atlasWidth = tileInfo.atlasWidth;
				sideTileInfo.atlasHeight = tileInfo.atlasHeight;
				for (int y = 0; y < tileTexture.GetHeight(); y++)
				{
					for (int x = 0; x < tileTexture.GetWidth(); x++)
					{
						uint32_t pixel = tileTexture.GetPixel(x, y);
						if ((pixel >> 24) != 0)
						{
							sideTileInfo.u0 = tileInfo.u0 + (float)x / (float)tileTexture.GetWidth() / ATLAS_TILE_SIZE;
							sideTileInfo.v0 = tileInfo.v0 + (float)y / (float)tileTexture.GetHeight() / ATLAS_TILE_SIZE;
							/*
							// Gray sides
							sideTileInfo.u0 = 1.0f - 1.f / 16.f;
							sideTileInfo.v0 = 1.0f - 1.f / 16.f;
							*/

							// Check pixel above
							if (y == 0 || (tileTexture.GetPixel(x, y - 1) >> 24) == 0)
							{
								float faceX = (float)x / tileTexture.GetWidth();
								float faceZ = (float)(y - 1) / tileTexture.GetHeight();

								ConstructItemSideMesh(mesh, FaceDirection::PosZ, sideTileInfo, faceX, faceZ, yScale, Vec3f(0.5f - 1.f / 16.f, -0.0f, 0.5f - 2.f / 16.f));
							}
							// Check pixel below
							if (y == tileTexture.GetHeight() - 1 || (tileTexture.GetPixel(x, y + 1) >> 24) == 0)
							{
								float faceX = (float)x / tileTexture.GetWidth();
								float faceZ = (float)(y + 1) / tileTexture.GetHeight();

								ConstructItemSideMesh(mesh, FaceDirection::NegZ, sideTileInfo, faceX, faceZ, yScale, Vec3f(0.5f - 1.f / 16.f, 0.0f, 0.5f));
							}
							// Check pixel left
							if (x == 0 || (tileTexture.GetPixel(x - 1, y) >> 24) == 0)
							{
								float faceX = (float)(x - 1) / tileTexture.GetWidth();
								float faceZ = (float)y / tileTexture.GetHeight();

								ConstructItemSideMesh(mesh, FaceDirection::PosX, sideTileInfo, faceX, faceZ, yScale, Vec3f(0.5f - 2.f / 16.f, 0.0f, 0.5f - 1.f / 16.f));
							}
							// Check pixel right
							if (x == tileTexture.GetWidth() - 1 || (tileTexture.GetPixel(x + 1, y) >> 24) == 0)
							{
								float faceX = (float)(x + 1) / tileTexture.GetWidth();
								float faceZ = (float)y / tileTexture.GetHeight();

								ConstructItemSideMesh(mesh, FaceDirection::NegX, sideTileInfo, faceX, faceZ, yScale, Vec3f(0.5f, 0.0f, 0.5f - 1.f / 16.f));
							}
						}
					}
				}

				m_Meshes.emplace(type, std::move(mesh));
			}
		}
	}

	ItemDefinition ItemRegistry::GetItemDefinition(ItemType type)
	{
		Texture* texture;
		if (IsItemFullBlock(type))
			texture = &m_TextureLoader.Get("atlas-old");
		else
			texture = &m_TextureLoader.Get("item-atlas");

		auto it = m_Meshes.find(type);
		if (it == m_Meshes.end())
		{
			return { type, nullptr, texture };
		}
		return { type, &it->second, texture };
	}

	Texture ItemRegistry::GetTextureForItemType(ItemType type) const
	{
		Texture* atlasTexture = &m_TextureLoader.Get("item-atlas");
		Texture texture(ITEM_TEXTURE_SIZE, ITEM_TEXTURE_SIZE);

		if (IsItemFullBlock(type))
			atlasTexture = &m_TextureLoader.Get("block-item-atlas");

		AtlasTileInfo tileInfo = GetAtlasTileInfoForItem(type);
		int width = tileInfo.uScale * tileInfo.atlasWidth;
		int height = tileInfo.vScale * tileInfo.atlasHeight;
		for (int y = 0; y < ITEM_TEXTURE_SIZE; y++)
		{
			for (int x = 0; x < ITEM_TEXTURE_SIZE; x++)
			{
				uint32_t pixel = atlasTexture->GetPixel(
					(int)(tileInfo.u0 * tileInfo.atlasWidth) + x * ((float)width / (float)ITEM_TEXTURE_SIZE),
					(int)(tileInfo.v0 * tileInfo.atlasHeight) + y * ((float)height / (float)ITEM_TEXTURE_SIZE)
				);
				texture.SetPixel(x, y, pixel);
			}
		}

		return texture;
	}

	AtlasTileInfo ItemRegistry::GetAtlasTileInfoForItem(ItemType type)
	{
		switch (type)
		{
		case ItemType::Rose:
			return CreateAtlasTileInfo(0, 0, false, true);
		case ItemType::YellowFlower:
			return CreateAtlasTileInfo(1, 0, false, true);

		// Block items
		case ItemType::Stone:
			return CreateAtlasTileInfo(0, 0, false, true, s_BlockItemAtlasWidth, s_BlockItemAtlasHeight, s_BlockItemAtlasTileSize);
		case ItemType::Grass:
			return CreateAtlasTileInfo(1, 0, false, true, s_BlockItemAtlasWidth, s_BlockItemAtlasHeight, s_BlockItemAtlasTileSize);
		case ItemType::Dirt:
			return CreateAtlasTileInfo(2, 0, false, true, s_BlockItemAtlasWidth, s_BlockItemAtlasHeight, s_BlockItemAtlasTileSize);
		case ItemType::Cobblestone:
			return CreateAtlasTileInfo(3, 0, false, true, s_BlockItemAtlasWidth, s_BlockItemAtlasHeight, s_BlockItemAtlasTileSize);
		case ItemType::OakPlanks:
			return CreateAtlasTileInfo(4, 0, false, true, s_BlockItemAtlasWidth, s_BlockItemAtlasHeight, s_BlockItemAtlasTileSize);
		case ItemType::Sand:
			return CreateAtlasTileInfo(6, 0, false, true, s_BlockItemAtlasWidth, s_BlockItemAtlasHeight, s_BlockItemAtlasTileSize);
		case ItemType::Gravel:
			return CreateAtlasTileInfo(7, 0, false, true, s_BlockItemAtlasWidth, s_BlockItemAtlasHeight, s_BlockItemAtlasTileSize);
		case ItemType::GoldOre:
			return CreateAtlasTileInfo(8, 0, false, true, s_BlockItemAtlasWidth, s_BlockItemAtlasHeight, s_BlockItemAtlasTileSize);
		case ItemType::IronOre:
			return CreateAtlasTileInfo(9, 0, false, true, s_BlockItemAtlasWidth, s_BlockItemAtlasHeight, s_BlockItemAtlasTileSize);
		case ItemType::CoalOre:
			return CreateAtlasTileInfo(0, 1, false, true, s_BlockItemAtlasWidth, s_BlockItemAtlasHeight, s_BlockItemAtlasTileSize);
		case ItemType::OakLog:
			return CreateAtlasTileInfo(1, 1, false, true, s_BlockItemAtlasWidth, s_BlockItemAtlasHeight, s_BlockItemAtlasTileSize);
		case ItemType::OakLeaves:
			return CreateAtlasTileInfo(2, 1, false, true, s_BlockItemAtlasWidth, s_BlockItemAtlasHeight, s_BlockItemAtlasTileSize);
		case ItemType::Glass:
			return CreateAtlasTileInfo(4, 1, false, true, s_BlockItemAtlasWidth, s_BlockItemAtlasHeight, s_BlockItemAtlasTileSize);
		case ItemType::GoldBlock:
			return CreateAtlasTileInfo(5, 1, false, true, s_BlockItemAtlasWidth, s_BlockItemAtlasHeight, s_BlockItemAtlasTileSize);
		case ItemType::IronBlock:
			return CreateAtlasTileInfo(6, 1, false, true, s_BlockItemAtlasWidth, s_BlockItemAtlasHeight, s_BlockItemAtlasTileSize);
		case ItemType::DoubleStoneSlab:
			return CreateAtlasTileInfo(7, 1, false, true, s_BlockItemAtlasWidth, s_BlockItemAtlasHeight, s_BlockItemAtlasTileSize);
		case ItemType::Brick:
			return CreateAtlasTileInfo(8, 1, false, true, s_BlockItemAtlasWidth, s_BlockItemAtlasHeight, s_BlockItemAtlasTileSize);
		case ItemType::TNT:
			return CreateAtlasTileInfo(9, 1, false, true, s_BlockItemAtlasWidth, s_BlockItemAtlasHeight, s_BlockItemAtlasTileSize);
		case ItemType::Bookshelf:
			return CreateAtlasTileInfo(0, 2, false, true, s_BlockItemAtlasWidth, s_BlockItemAtlasHeight, s_BlockItemAtlasTileSize);
		case ItemType::Obsidian:
			return CreateAtlasTileInfo(1, 2, false, true, s_BlockItemAtlasWidth, s_BlockItemAtlasHeight, s_BlockItemAtlasTileSize);
		default:
			return {};
		}
	}

	BlockType ItemRegistry::GetBlockTypeFromItemType(ItemType type)
	{
		switch (type)
		{
		case ItemType::Grass:            return BlockType::Grass;
		case ItemType::Dirt:             return BlockType::Dirt;
		case ItemType::Stone:            return BlockType::Stone;
		case ItemType::OakPlanks:        return BlockType::OakPlanks;
		case ItemType::DoubleStoneSlab:  return BlockType::DoubleStoneSlab;
		case ItemType::PolishedStone:    return BlockType::PolishedStone;
		case ItemType::Brick:            return BlockType::Brick;
		case ItemType::TNT:              return BlockType::TNT;
		case ItemType::Cobblestone:      return BlockType::Cobblestone;
		case ItemType::Sand:             return BlockType::Sand;
		case ItemType::Gravel:           return BlockType::Gravel;
		case ItemType::OakLog:           return BlockType::OakLog;
		case ItemType::OakLeaves:        return BlockType::OakLeaves;
		case ItemType::IronBlock:        return BlockType::IronBlock;
		case ItemType::GoldBlock:        return BlockType::GoldBlock;
		case ItemType::GoldOre:          return BlockType::GoldOre;
		case ItemType::IronOre:          return BlockType::IronOre;
		case ItemType::CoalOre:          return BlockType::CoalOre;
		case ItemType::Bookshelf:        return BlockType::Bookshelf;
		case ItemType::Glass:            return BlockType::Glass;
		case ItemType::Obsidian:         return BlockType::Obsidian;

		case ItemType::Rose:			 return BlockType::Rose;
		case ItemType::YellowFlower:	 return BlockType::YellowFlower;
		default:                         return BlockType::Undefined;
		}
	}

	ItemType ItemRegistry::GetItemTypeFromBlockType(BlockType type)
	{
		switch (type)
		{
		case BlockType::Grass:            return ItemType::Grass;
		case BlockType::Dirt:             return ItemType::Dirt;
		case BlockType::Stone:            return ItemType::Stone;
		case BlockType::OakPlanks:        return ItemType::OakPlanks;
		case BlockType::DoubleStoneSlab:  return ItemType::DoubleStoneSlab;
		case BlockType::PolishedStone:    return ItemType::PolishedStone;
		case BlockType::Brick:            return ItemType::Brick;
		case BlockType::TNT:              return ItemType::TNT;
		case BlockType::Cobblestone:      return ItemType::Cobblestone;
		case BlockType::Sand:             return ItemType::Sand;
		case BlockType::Gravel:           return ItemType::Gravel;
		case BlockType::OakLog:           return ItemType::OakLog;
		case BlockType::OakLeaves:        return ItemType::OakLeaves;
		case BlockType::IronBlock:        return ItemType::IronBlock;
		case BlockType::GoldBlock:        return ItemType::GoldBlock;
		case BlockType::GoldOre:          return ItemType::GoldOre;
		case BlockType::IronOre:          return ItemType::IronOre;
		case BlockType::CoalOre:          return ItemType::CoalOre;
		case BlockType::Bookshelf:        return ItemType::Bookshelf;
		case BlockType::Glass:            return ItemType::Glass;
		case BlockType::Obsidian:         return ItemType::Obsidian;
		
		case BlockType::Rose:			  return ItemType::Rose;
		case BlockType::YellowFlower:	  return ItemType::YellowFlower;
		default:                          return ItemType::Undefined;
		}
	}

	uint8_t ItemRegistry::GetStackSizeForItemType(ItemType type)
	{
		switch (type)
		{
		default: return 64;
		};
	}

	bool ItemRegistry::IsItemFullBlock(ItemType type)
	{
		return Block(GetBlockTypeFromItemType(type)).IsFullBlock();
	}

	bool ItemRegistry::IsItemPlacable(ItemType type)
	{
		if (IsItemFullBlock(type)) return true;

		switch (type)
		{
		case ItemType::Rose:
		case ItemType::YellowFlower:
			return true;
		default:
			return false;
		}
	}

	/* Private functions */

	void ItemRegistry::ConstructItemSideMesh(
		Mesh& mesh, FaceDirection dir, AtlasTileInfo tileInfo, 
		float faceX, float faceZ, float yScale, Vec3f offset
	) const
	{
		int startIndex = mesh.vertices.size();

		BlockFace face = GetBlockFace(BlockType::Stone, dir);
		for (int i = 0; i < 4; i++)
		{
			Vertex v;
			v.position = Vec3f(
				face.vertices[i].x,
				face.vertices[i].y,
				face.vertices[i].z
			);

			v.position.x /= static_cast<float>(ATLAS_TILE_SIZE);
			v.position.z /= static_cast<float>(ATLAS_TILE_SIZE);

			v.position.x -= faceX;
			v.position.z -= faceZ;

			v.position.y *= yScale;
			v.position *= m_ItemScaleFactor;
			v.position.y -= m_ItemHitboxSize.y * m_ItemScaleFactor;

			v.position += offset * m_ItemScaleFactor;

			v.normal = face.normal;

			v.u = face.uvs[i].x;
			v.v = face.uvs[i].y;

			v.tileInfo = tileInfo;

			mesh.vertices.push_back(v);
		}

		mesh.indices.push_back(startIndex + 0);
		mesh.indices.push_back(startIndex + 1);
		mesh.indices.push_back(startIndex + 2);

		mesh.indices.push_back(startIndex + 0);
		mesh.indices.push_back(startIndex + 2);
		mesh.indices.push_back(startIndex + 3);
	}
}