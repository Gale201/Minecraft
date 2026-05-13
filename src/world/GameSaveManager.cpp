#include "GameSaveManager.h"
#include <filesystem>

namespace Minecraft
{
	GameSaveManager::GameSaveManager(
		const std::string& worldFolderPath,
		const std::string& worldName,
		ItemRegistry& itemRegistry
	) :
		m_BaseFolderPath(worldFolderPath),
		m_WorldName(worldName), 
		m_WorldFolderPath(worldFolderPath),
		m_ItemRegistry(itemRegistry)
	{
		if (m_WorldFolderPath.back() != '/' && m_WorldFolderPath.back() != '\\')
		{
			m_WorldFolderPath += '/';
		}
		m_WorldFolderPath += worldName + "\\";
	}

	/* World saving and loading */

	std::unique_ptr<World> GameSaveManager::CreateWorld(unsigned int seed) const
	{
		for (const auto& entry : std::filesystem::directory_iterator(m_BaseFolderPath))
		{
			if (entry.is_directory() && entry.path().filename() == m_WorldName) return nullptr;
		}
		std::filesystem::create_directories(m_WorldFolderPath);
		return std::make_unique<World>(m_WorldName, seed, m_ItemRegistry);
	}

	std::unique_ptr<World> GameSaveManager::LoadWorld() const
	{
		const std::string worldFilePath = m_WorldFolderPath + m_WorldSaveFileName;

		std::ifstream in(worldFilePath, std::ios::binary);
		if (!in) return nullptr;

		// Reading seed
		unsigned int seed;
		in.read(reinterpret_cast<char*>(&seed), sizeof(seed));

		// Reading chunk count
		int chunkCount;
		in.read(reinterpret_cast<char*>(&chunkCount), sizeof(chunkCount));

		std::unordered_map<Vec3i, ModifiedChunk, Vec3iHash> modifiedChunks;
		modifiedChunks.reserve(chunkCount);

		for (int i = 0; i < chunkCount; i++)
		{
			ModifiedChunk chunk;
			// Reading chunk position
			Vec3i chunkPos;
			in.read(reinterpret_cast<char*>(&chunkPos), sizeof(chunkPos));
			modifiedChunks.emplace(chunkPos, std::move(chunk));

			// Reading block count
			int blockCount;
			in.read(reinterpret_cast<char*>(&blockCount), sizeof(blockCount));
			modifiedChunks.at(chunkPos).blocks.reserve(blockCount);

			for (int j = 0; j < blockCount; j++)
			{
				// Reading block position and type
				Vec3i pos;
				BlockType type;
				uint8_t fluidLevel;
				in.read(reinterpret_cast<char*>(&pos), sizeof(pos));
				in.read(reinterpret_cast<char*>(&type), sizeof(type));
				in.read(reinterpret_cast<char*>(&fluidLevel), sizeof(fluidLevel));

				modifiedChunks.at(chunkPos).blocks.emplace(pos, Block(type, fluidLevel));
			}
		}

		return std::make_unique<World>(m_WorldName, seed, modifiedChunks, m_ItemRegistry);
	}

	void GameSaveManager::SaveWorld(const World& world) const
	{
		const std::string worldFilePath = m_WorldFolderPath + m_WorldSaveFileName;

		std::filesystem::create_directories(m_WorldFolderPath);

		std::ofstream out(worldFilePath, std::ios::binary);
		if (!out) return;

		// Writing seed
		unsigned int seed = world.GetSeed();
		out.write(reinterpret_cast<const char*>(&seed), sizeof(seed));

		// Writing chunk count
		int chunkCount = static_cast<int>(world.GetModifiedChunks().size());
		out.write(reinterpret_cast<const char*>(&chunkCount), sizeof(chunkCount));

		for (const auto& [chunkPos, modifiedChunk] : world.GetModifiedChunks())
		{
			// Writing chunk position
			out.write(reinterpret_cast<const char*>(&chunkPos), sizeof(chunkPos));

			// Writing block count
			int blockCount = static_cast<int>(modifiedChunk.blocks.size());
			out.write(reinterpret_cast<const char*>(&blockCount), sizeof(blockCount));

			for (const auto& [blockPos, block] : modifiedChunk.blocks)
			{
				// Writing block position and type
				out.write(reinterpret_cast<const char*>(&blockPos), sizeof(blockPos));
				out.write(reinterpret_cast<const char*>(&block.type), sizeof(block.type));
				out.write(reinterpret_cast<const char*>(&block.fluidLevel), sizeof(block.fluidLevel));
			}
		}
	}

	bool GameSaveManager::WorldExists() const
	{
		for (const auto& entry : std::filesystem::directory_iterator(m_BaseFolderPath))
		{
			if (entry.is_directory() && entry.path().filename().string() == m_WorldName) return true;
		}
		return false;
	}

	/* Player saving and loading */

	PlayerSaveData GameSaveManager::LoadPlayerData() const
	{
		const std::string playerFilePath = m_WorldFolderPath + m_PlayerSaveFileName;

		std::ifstream in(playerFilePath, std::ios::binary);
		if (!in) return {};

		PlayerSaveData playerData;
		in.read(reinterpret_cast<char*>(&playerData.position), sizeof(playerData.position));
		in.read(reinterpret_cast<char*>(&playerData.headRotation), sizeof(playerData.headRotation));
		in.read(reinterpret_cast<char*>(&playerData.selectedHotbarSlotIndex), sizeof(playerData.selectedHotbarSlotIndex));

		for (int i = 0; i < Inventory::s_Size; i++)
		{
			in.read(reinterpret_cast<char*>(&playerData.inventorySlots[i]), sizeof(ItemStack));
		}

		return playerData;
	}

	void GameSaveManager::SavePlayer(const Player& player) const
	{
		const std::string playerFilePath = m_WorldFolderPath + m_PlayerSaveFileName;

		std::ofstream out(playerFilePath, std::ios::binary);
		if (!out) return;

		PlayerSaveData playerData = {
			player.GetPosition(),
			player.GetHeadRotation(),
			player.GetSelectedHotbarSlotIndex(),
		};
		out.write(reinterpret_cast<const char*>(&playerData.position), sizeof(playerData.position));
		out.write(reinterpret_cast<const char*>(&playerData.headRotation), sizeof(playerData.headRotation));
		out.write(reinterpret_cast<const char*>(&playerData.selectedHotbarSlotIndex), sizeof(playerData.selectedHotbarSlotIndex));

		for (const ItemStack& stack : player.GetInventory().GetSlots())
		{
			out.write(reinterpret_cast<const char*>(&stack), sizeof(stack));
		}
	}

	/* Item saving and loading */

	void GameSaveManager::LoadWorldItems(World& world, PhysicsSystem& physicsSystem) const
	{
		const std::string itemsFilePath = m_WorldFolderPath + "items.data";
		
		std::ifstream in(itemsFilePath, std::ios::binary);
		if (!in) return;
		
		int chunkCount;
		in.read(reinterpret_cast<char*>(&chunkCount), sizeof(chunkCount));
		
		for (int i = 0; i < chunkCount; i++)
		{
			Vec3i chunkPos;
			in.read(reinterpret_cast<char*>(&chunkPos), sizeof(chunkPos));
			
			int itemCount;
			in.read(reinterpret_cast<char*>(&itemCount), sizeof(itemCount));
			
			for (int j = 0; j < itemCount; j++)
			{
				Item item;
				PhysicsBody body;
				in.read(reinterpret_cast<char*>(&item), sizeof(item));
				in.read(reinterpret_cast<char*>(&body), sizeof(body));

				world.AddItem(body, item.stack, item.rotation.y);
			}
		}
	}

	void GameSaveManager::SaveWorldItems(const World& world) const
	{
		const std::string itemsFilePath = m_WorldFolderPath + "items.data";
		
		std::ofstream out(itemsFilePath, std::ios::binary);
		if (!out) return;

		const auto& items = world.GetItems();
		const auto& itemsByChunk = world.GetItemsByChunk();

		int chunkCount = static_cast<int>(itemsByChunk.size());
		out.write(reinterpret_cast<const char*>(&chunkCount), sizeof(chunkCount));

		for (const auto& [chunkPos, indices] : itemsByChunk)
		{
			out.write(reinterpret_cast<const char*>(&chunkPos), sizeof(chunkPos));

			int itemCount = static_cast<int>(indices.size());
			out.write(reinterpret_cast<const char*>(&itemCount), sizeof(itemCount));

			for (size_t index : indices)
			{
				const Item& item = items[index];
				PhysicsBody& body = *item.body;

				out.write(reinterpret_cast<const char*>(&item), sizeof(item));
				out.write(reinterpret_cast<const char*>(&body), sizeof(body));
			}
		}
	}
}