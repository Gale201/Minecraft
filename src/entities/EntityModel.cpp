#include "EntityModel.h"

namespace Minecraft
{
	void EntityModel::Render(Renderer& renderer, const Camera& camera)
	{
		if (!m_Texture) return;

		Mat4 bodyScale = Mat4::Scale(m_Scale);
		Mat4 pivotMove = Mat4::Translate(m_Pivot);
		Mat4 bodyRotate = Mat4::RotateXYZ(m_Rotation);
		Mat4 pivotMoveBack = Mat4::Translate(-m_Pivot);
		Mat4 bodyTranslate = Mat4::Translate(m_Position);

		Mat4 bodyMatrix = bodyScale * pivotMove * bodyRotate * pivotMoveBack * bodyTranslate;

		for (const auto& [name, part] : m_Parts)
		{
			Transform transform;

			transform.SetPosition(part.position + part.pivot);
			transform.SetRotation(part.rotation);
			transform.SetScale(part.scale);

			transform.SetParentMatrix(bodyMatrix);

			renderer.DrawMesh(*part.mesh, transform, *m_Texture, camera, {}, false);
		}
	}

	void EntityModel::RenderPart(Renderer& renderer, const Camera& camera, const std::string& partName)
	{
		if (!m_Texture) return;
		if (m_Parts.find(partName) == m_Parts.end()) return;

		const EntityModelPart& part = m_Parts.at(partName);
		Transform transform;
		transform.SetScale(
			Vec3f(m_Scale.x * part.scale.x,
				m_Scale.y * part.scale.y,
				m_Scale.z * part.scale.z)
		);
		Vec3f pivotOffset = Vec3f(
			part.pivot.x * transform.GetScale().x,
			part.pivot.y * transform.GetScale().y,
			part.pivot.z * transform.GetScale().z
		);
		transform.SetPosition(m_Position + part.position + pivotOffset);
		transform.SetRotation(m_Rotation + part.rotation);
		renderer.DrawMesh(*part.mesh, transform, *m_Texture, camera, {}, false);
	}

	EntityModelPart* EntityModel::GetPart(const std::string& name)
	{
		if (m_Parts.find(name) == m_Parts.end())
			return nullptr;
		return &m_Parts.at(name);
	}

	Vec3f EntityModel::GetAbsolutePartPosition(const std::string& partName) const
	{
		if (m_Parts.find(partName) == m_Parts.end())
			return Vec3f(0.0f, 0.0f, 0.0f);
		return m_Position + m_Parts.at(partName).position;
	}

	Vec3f EntityModel::GetAbsolutePartRotation(const std::string& partName) const
	{
		if (m_Parts.find(partName) == m_Parts.end())
			return Vec3f(0.0f, 0.0f, 0.0f);
		return m_Rotation + m_Parts.at(partName).rotation;
	}

	Vec3f EntityModel::GetAbsolutePartScale(const std::string& partName) const
	{
		if (m_Parts.find(partName) == m_Parts.end())
			return Vec3f(1.0f, 1.0f, 1.0f);
		return Vec3f(
			m_Scale.x * m_Parts.at(partName).scale.x,
			m_Scale.y * m_Parts.at(partName).scale.y,
			m_Scale.z * m_Parts.at(partName).scale.z
		);
	}
}