#include "ItemRegistry.h"

namespace Minecraft
{
	void ItemRegistry::Initialize()
	{
		for (ItemType type : m_ItemTypes)
		{
			if (ItemStack::IsItemBlock(type))
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

				FaceDirection dirs[2] = { FaceDirection::PosY, FaceDirection::NegY };
				for (int i = 0; i < 2; i++)
				{
					int startIndex = mesh.vertices.size();
					BlockFace face = GetBlockFace(BlockType::Dirt, dirs[i]);
					for (int i = 0; i < 4; i++)
					{
						Vertex v;
						v.position = Vec3f(
							face.vertices[i].x - 0.5f,
							face.vertices[i].y - 0.5f,
							face.vertices[i].z - 0.5f
						) * m_ItemScaleFactor;
						v.position.y *= 0.1f;
						v.position.y -= 0.1f;
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
				
				int u = tileInfo.u0 * tileInfo.atlasWidth;
				int v = tileInfo.v0 * tileInfo.atlasHeight;
				int w = tileInfo.uScale * tileInfo.atlasWidth;
				int h = tileInfo.vScale * tileInfo.atlasHeight;
				Texture tileTexture = m_TextureLoader.Get("item-atlas").Crop(u, v, w, h);

				for (int y = 0; y < tileTexture.GetHeight(); y++)
				{
					for (int x = 0; x < tileTexture.GetWidth(); x++)
					{
						uint32_t pixel = tileTexture.GetPixel(x, y);
						if ((pixel >> 24) != 0)
						{
							// Check pixel above
							if (y > 0 && (tileTexture.GetPixel(x, y - 1) >> 24) == 0)
							{
								int faceX = x / tileTexture.GetWidth();
								int faceZ = (y - 1) / tileTexture.GetHeight();

								int startIndex = mesh.vertices.size();

								BlockFace face = GetBlockFace(BlockType::Dirt, FaceDirection::PosZ);
								for (int i = 0; i < 4; i++)
								{
									Vertex v;
									v.position = Vec3f(
										face.vertices[i].x,
										face.vertices[i].y,
										face.vertices[i].z
									) * m_ItemScaleFactor;
									v.position.y *= 0.1f;
									v.position.y -= 0.1f;
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
							// Check pixel below
							if (y < tileTexture.GetHeight() - 1 && (tileTexture.GetPixel(x, y + 1) >> 24) == 0)
							{
							}
							// Check pixel left
							if (x > 0 && (tileTexture.GetPixel(x - 1, y) >> 24) == 0)
							{
							}
							// Check pixel right
							if (x < tileTexture.GetWidth() - 1 && (tileTexture.GetPixel(x + 1, y) >> 24) == 0)
							{
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
		if (ItemStack::IsItemBlock(type))
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

	AtlasTileInfo ItemRegistry::GetAtlasTileInfoForItem(ItemType type)
	{
		switch (type)
		{
		case ItemType::Rose:
			return CreateAtlasTileInfo(0, 0, false, true);
		case ItemType::YellowFlower:
			return CreateAtlasTileInfo(1, 0, false, true);
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

		case ItemType::Rose:			 return BlockType::Rose;
		case ItemType::YellowFlower:	 return BlockType::YellowFlower;
		default:                         return BlockType::Air;
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
		
		case BlockType::Rose:			  return ItemType::Rose;
		case BlockType::YellowFlower:	  return ItemType::YellowFlower;
		default:                          return ItemType::Undefined;
		}
	}
}