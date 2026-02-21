#include "GameSaveManager.h"
#include <filesystem>

namespace Minecraft
{
	GameSaveManager::GameSaveManager(const std::string& worldFolderPath, const std::string& worldName) :
		m_BaseFolderPath(worldFolderPath), m_WorldName(worldName), m_WorldFolderPath(worldFolderPath)
	{
		if (m_WorldFolderPath.back() != '/' && m_WorldFolderPath.back() != '\\')
		{
			m_WorldFolderPath += '/';
		}
		m_WorldFolderPath += worldName + "\\";

		std::filesystem::create_directories(m_WorldFolderPath);
	}

	/* World saving and loading */

	std::unique_ptr<World> GameSaveManager::CreateWorld(unsigned int seed) const
	{
		for (const auto& entry : std::filesystem::directory_iterator(m_WorldFolderPath))
		{
			if (entry.is_directory() && entry.path().filename() == m_WorldName) return nullptr;
		}
		return std::make_unique<World>(m_WorldName, seed);
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
				in.read(reinterpret_cast<char*>(&pos), sizeof(pos));
				in.read(reinterpret_cast<char*>(&type), sizeof(type));

				modifiedChunks.at(chunkPos).blocks.emplace(pos, type);
			}
		}

		return std::make_unique<World>(m_WorldName, seed, modifiedChunks);
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

			for (const auto& [blockPos, blockType] : modifiedChunk.blocks)
			{
				// Writing block position and type
				out.write(reinterpret_cast<const char*>(&blockPos), sizeof(blockPos));
				out.write(reinterpret_cast<const char*>(&blockType), sizeof(blockType));
			}
		}
	}

	bool GameSaveManager::WorldExists() const
	{
		for (const auto& entry : std::filesystem::directory_iterator(m_BaseFolderPath))
		{
			if (entry.is_directory() && entry.path().filename() == m_WorldName) return true;
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

		return playerData;
	}

	void GameSaveManager::SavePlayer(const Player& player) const
	{
		const std::string playerFilePath = m_WorldFolderPath + m_PlayerSaveFileName;

		std::ofstream out(playerFilePath, std::ios::binary);
		if (!out) return;

		PlayerSaveData playerData = { player.GetPosition(), player.GetHeadRotation() };
		out.write(reinterpret_cast<const char*>(&playerData.position), sizeof(playerData.position));
		out.write(reinterpret_cast<const char*>(&playerData.headRotation), sizeof(playerData.headRotation));
	}
}