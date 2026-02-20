#pragma once

#include <string>
#include <memory>
#include <fstream>

#include "World.h"

namespace Minecraft
{
	class WorldLoader
	{
	public:
		WorldLoader(const std::string& worldFolderPath) :
			m_WorldFolderPath(worldFolderPath)
		{
			if (m_WorldFolderPath.back() != '/' && m_WorldFolderPath.back() != '\\')
			{
				m_WorldFolderPath += '/';
			}
		}

		std::unique_ptr<World> CreateWorld(const std::string& name, unsigned int seed) const;
		std::unique_ptr<World> LoadWorld(const std::string& name) const;
		void SaveWorld(const World& world) const;

		const std::string& GetWorldFolderPath() const { return m_WorldFolderPath; }
	
	private:
		struct PlayerSaveData
		{
			Vec3i position;
			Vec3i headRotation;
		};

	private:
		std::string m_WorldFolderPath;

		const std::string m_ChunkSavePath = "chunks.data";
		const std::string m_PlayerSavePath = "player.data";
	};
}
