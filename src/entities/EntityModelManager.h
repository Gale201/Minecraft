#pragma once

#include <unordered_map>

#include "EntityModel.h"
#include "../loaders/ModelLoader.h"

namespace Minecraft
{
	class EntityModelManager
	{
	public:
		EntityModelManager(const ModelLoader& modelLoader) : m_ModelLoader(modelLoader) {}
	
		bool LoadAllModels();

		EntityModel CreateEntityModel(const std::string& name) const;

	private:
		std::unordered_map<std::string, Model> m_Models;

		const ModelLoader& m_ModelLoader;

		const std::unordered_map<std::string, std::string> m_ModelFileNames = 
		{
			{ "player", "player.obj" }
		};
	};
}
