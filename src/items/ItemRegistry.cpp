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
						) * 0.25f;
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
		}
	}

	ItemDefinition ItemRegistry::GetItemDefinition(ItemType type)
	{
		Texture* texture;
		switch (type)
		{
		case ItemType::Dirt:
		case ItemType::Grass:
		case ItemType::Stone:
		case ItemType::Glass:
			texture = &m_TextureLoader.Get("atlas-old");
			break;
		default:
			texture = &m_TextureLoader.Get("missing");
			break;
		}

		auto it = m_Meshes.find(type);
		if (it == m_Meshes.end())
		{
			return { type, nullptr, texture };
		}
		return { type, &it->second, texture };
	}

	BlockType ItemRegistry::GetBlockTypeFromItemType(ItemType type)
	{
		switch (type)
		{
		case ItemType::Grass:
			return BlockType::Grass;
		case ItemType::Dirt:
			return BlockType::Dirt;
		case ItemType::Stone:
			return BlockType::Stone;
		case ItemType::Glass:
			return BlockType::Glass;
		default:
			return BlockType::Air;
		}
	}

	ItemType ItemRegistry::GetItemTypeFromBlockType(BlockType type)
	{
		switch (type)
		{
		case BlockType::Grass:
			return ItemType::Grass;
		case BlockType::Dirt:
			return ItemType::Dirt;
		case BlockType::Stone:
			return ItemType::Stone;
		case BlockType::Glass:
			return ItemType::Glass;
		default:
			return ItemType::Undefined;
		}
	}
}