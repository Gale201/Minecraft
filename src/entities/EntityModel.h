#pragma once

#include <unordered_map>

#include "EntityModelPart.h"
#include "../render/Renderer.h"

namespace Minecraft
{
	class EntityModel
	{
	public:
		EntityModel() {};

		void Render(Renderer& renderer, const Camera& camera);
		void RenderPart(Renderer& renderer, const Camera& camera, const std::string& partName);

		void AddPart(const std::string& name, const EntityModelPart& part)
		{
			m_Parts.emplace(name, part);
		}

		// Getters & setters
		EntityModelPart* GetPart(const std::string& name);

		Vec3f GetPosition() const { return m_Position; }
		Vec3f GetScale() const { return m_Scale; }
		Vec3f GetRotation() const { return m_Rotation; }

		void SetPosition(const Vec3f& position) { m_Position = position; }
		void SetScale(const Vec3f& scale) { m_Scale = scale; }
		void SetRotation(const Vec3f& rotation) { m_Rotation = rotation; }

		void SetTexture(Texture& texture)
		{
			m_Texture = &texture;
		}

	private:
		Vec3f m_Position;
		Vec3f m_Scale = Vec3f(1.0f, 1.0f, 1.0f);
		Vec3f m_Rotation;

		Texture* m_Texture = nullptr;

		std::unordered_map<std::string, EntityModelPart> m_Parts;
	};
}
