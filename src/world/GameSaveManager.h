#pragma once

#include <string>
#include <memory>
#include <fstream>

#include "World.h"
#include "../player/Player.h"
#include "../items/ItemRegistry.h"
#include "../physics/PhysicsSystem.h"

namespace Minecraft
{
	struct PlayerSaveData
	{
		Vec3f position;
		Vec3f headRotation;
		int selectedHotbarSlotIndex;
		std::array<ItemStack, Inventory::s_Size> inventorySlots;
	};

	class GameSaveManager
	{
	public:
		GameSaveManager(
			const std::string& worldsFolderPath, 
			const std::string& worldName,
			ItemRegistry& itemRegistry
		);

		// World loading/saving
		std::unique_ptr<World> CreateWorld(unsigned int seed) const;
		std::unique_ptr<World> LoadWorld() const;
		void SaveWorld(const World& world) const;

		bool WorldExists() const;

		// Player loading/saving
		PlayerSaveData LoadPlayerData() const;
		void SavePlayer(const Player& player) const;

		// Item loading/saving
		void LoadWorldItems(World& world, PhysicsSystem& physicsSystem) const;
		void SaveWorldItems(const World& world) const;

		// Getters
		const std::string& GetWorldFolderPath() const { return m_WorldFolderPath; }

	private:
		std::string m_BaseFolderPath;
		std::string m_WorldFolderPath;
		std::string m_WorldName;

		const std::string m_WorldSaveFileName = "world.data";
		const std::string m_PlayerSaveFileName = "player.data";

		ItemRegistry& m_ItemRegistry;
	};
}
