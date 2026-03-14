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

		static AtlasTileInfo GetAtlasTileInfoForItem(ItemType type);

		static BlockType GetBlockTypeFromItemType(ItemType type);
		static ItemType GetItemTypeFromBlockType(BlockType type);
	private:
		TextureLoader& m_TextureLoader;

		std::vector<ItemType> m_ItemTypes{ 
			ItemType::Grass, 
			ItemType::Dirt, 
			ItemType::Stone,
			ItemType::OakPlanks,
			ItemType::DoubleStoneSlab,
			ItemType::PolishedStone,
			ItemType::Brick,
			ItemType::TNT,
			ItemType::Cobblestone,
			ItemType::Sand,
			ItemType::Gravel,
			ItemType::OakLog,
			ItemType::OakLeaves,
			ItemType::IronBlock,
			ItemType::GoldBlock,
			ItemType::GoldOre,
			ItemType::IronOre,
			ItemType::CoalOre,
			ItemType::Bookshelf,
			ItemType::Glass,

			ItemType::Rose,
			ItemType::YellowFlower
		};
		std::unordered_map<ItemType, Mesh> m_Meshes;

		float m_ItemScaleFactor = 0.5f;
		float m_BlockItemScaleFactor = 0.25f;
	};
}