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
		Texture GetTextureForItemType(ItemType type) const;
		
		std::unordered_map<ItemType, Mesh>& GetMeshes() { return m_Meshes; }

		static AtlasTileInfo GetAtlasTileInfoForItem(ItemType type);

		static BlockType GetBlockTypeFromItemType(ItemType type);
		static ItemType GetItemTypeFromBlockType(BlockType type);

		static uint8_t GetStackSizeForItemType(ItemType type);
		static bool IsItemFullBlock(ItemType type);
		static bool IsItemPlacable(ItemType type);

		Vec3f GetItemHitboxSize() const { return m_ItemHitboxSize; }
	private:
		void ConstructItemSideMesh(Mesh& mesh, FaceDirection dir, AtlasTileInfo tileInfo, float faceX, float faceZ, float yScale, Vec3f offset) const;

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
			ItemType::Obsidian,

			ItemType::Rose,
			ItemType::YellowFlower
		};
		std::unordered_map<ItemType, Mesh> m_Meshes;

		float m_ItemScaleFactor = 0.5f;
		float m_BlockItemScaleFactor = 0.25f;

		static constexpr float s_BlockItemAtlasWidth = 1000.f;
		static constexpr float s_BlockItemAtlasHeight = 1000.f;
		static constexpr float s_BlockItemAtlasTileSize = 100.f;

		const Vec3f m_ItemHitboxSize = Vec3f(0.3f, 0.25f, 0.3f);
	};
}