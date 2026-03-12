#pragma once

#include <unordered_map>

#include "ItemStack.h"

#include "../render/Mesh.h"
#include "../render/Texture.h"
#include "../loaders/TextureLoader.h"
#include "../world/Block.h"

namespace Minecraft
{
	struct ItemDefinition
	{
		ItemType type;
		Mesh* mesh;
		Texture* texture;
	};

	class ItemRegistry
	{
	public:
		ItemRegistry(TextureLoader& textureLoader) 
			: m_TextureLoader(textureLoader) 
		{
		}

		void Initialize();

		ItemDefinition GetItemDefinition(ItemType type);
		std::unordered_map<ItemType, Mesh>& GetMeshes() { return m_Meshes; }

		static BlockType GetBlockTypeFromItemType(ItemType type);
		static ItemType GetItemTypeFromBlockType(BlockType type);
	private:
		TextureLoader& m_TextureLoader;

		std::vector<ItemType> m_ItemTypes{ 
			ItemType::Grass, 
			ItemType::Dirt, 
			ItemType::Stone, 
			ItemType::Glass, 
			ItemType::Rose
		};
		std::unordered_map<ItemType, Mesh> m_Meshes;
	};
}