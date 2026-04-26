#include "EntityModel.h"

namespace Minecraft
{
	void EntityModel::Render(Renderer& renderer, const Camera& camera)
	{
		if (!m_Texture) return;

		Transform transform;
		for (const auto& [name, part] : m_Parts)
		{
			transform.SetPosition(m_Position + part.position);
			transform.SetScale(
				Vec3f(m_Scale.x * part.scale.x,
					m_Scale.y * part.scale.y,
					m_Scale.z * part.scale.z)
			);
			transform.SetRotation(m_Rotation + part.rotation);
			renderer.DrawMesh(*part.mesh, transform, *m_Texture, camera);
		}
	}

	void EntityModel::RenderPart(Renderer& renderer, const Camera& camera, const std::string& partName)
	{
		if (!m_Texture) return;
		if (m_Parts.find(partName) == m_Parts.end()) return;

		const EntityModelPart& part = m_Parts.at(partName);
		Transform transform;
		transform.SetPosition(m_Position + part.position);
		transform.SetScale(
			Vec3f(m_Scale.x * part.scale.x,
				m_Scale.y * part.scale.y,
				m_Scale.z * part.scale.z)
		);
		transform.SetRotation(m_Rotation + part.rotation);
		renderer.DrawMesh(*part.mesh, transform, *m_Texture, camera);
	}

	EntityModelPart* EntityModel::GetPart(const std::string& name)
	{
		if (m_Parts.find(name) == m_Parts.end())
			return nullptr;
		return &m_Parts.at(name);
	}
}