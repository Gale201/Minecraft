#include "EntityModelManager.h"
#include "../core/Log.h"

namespace Minecraft
{
	bool EntityModelManager::LoadAllModels()
	{
		for (const auto& [name, filename] : m_ModelFileNames)
		{
			auto model = m_ModelLoader.LoadModelFromObj(filename);
			if (!model.has_value())
			{
				LOG_ERROR("Error loading model %s from file %s", name.c_str(), filename.c_str());
				return false;
			}

			m_Models.emplace(name, std::move(model.value()));
		}
	}

	EntityModel EntityModelManager::CreateEntityModel(const std::string& name) const
	{
		if (m_Models.find(name) == m_Models.end())
		{
			LOG_ERROR("Error creating entity model. Model with name %s doesn't exist.", name.c_str());
			return EntityModel();
		}

		EntityModel entityModel;
		const Model& model = m_Models.at(name);
		for (size_t i = 0; i < model.meshes.size(); i++)
		{
			EntityModelPart part;
			part.mesh = &model.meshes[i];
			entityModel.AddPart(model.meshNames[i], part);
		}
		return entityModel;
	}
}