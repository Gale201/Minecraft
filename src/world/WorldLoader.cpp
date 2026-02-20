#include "WorldLoader.h"
#include <filesystem>

namespace Minecraft
{
	std::unique_ptr<World> WorldLoader::CreateWorld(const std::string& name, unsigned int seed) const
	{
		for (const auto& entry : std::filesystem::directory_iterator(m_WorldFolderPath))
		{
			if (entry.is_directory() && entry.path().filename() == name) return nullptr;
		}
		return std::make_unique<World>(name, seed);
	}

	std::unique_ptr<World> WorldLoader::LoadWorld(const std::string& name) const
	{
		auto world = std::make_unique<World>(name, 0);
		return world;
	}

	void WorldLoader::SaveWorld(const World& world) const
	{
		const std::string outputFolderPath = m_WorldFolderPath + world.GetName() + "\\";
		const std::string chunkFilePath = outputFolderPath + m_ChunkSavePath;
		const std::string playerFilePath = outputFolderPath + m_PlayerSavePath;

		std::ofstream out(chunkFilePath, std::ios::binary);
		if (!out) return;
	}
}