#pragma once
#include "../math/Math.h"

namespace Minecraft
{
	class AABB
	{
	public:
		AABB(float minX, float maxX, float minY, float maxY, float minZ, float maxZ) :
			m_MinX(minX), m_MaxX(maxX), m_MinY(minY), m_MaxY(maxY), m_MinZ(minZ), m_MaxZ(maxZ)
		{
		}
		AABB(Vec3f position, Vec3f size) :
			m_MinX(position.x), m_MaxX(position.x + size.x),
			m_MinY(position.y), m_MaxY(position.y + size.y),
			m_MinZ(position.z), m_MaxZ(position.z + size.z)
		{
		}

		AABB Expand(Vec3f value) const;
		AABB Grow(Vec3f value) const;
		void Move(Vec3f value);

		bool Intersects(AABB other) const;
		float GetClipX(AABB other, float delta) const;
		float GetClipY(AABB other, float delta) const;
		float GetClipZ(AABB other, float delta) const;

		Vec3f GetPosition() const;
		Vec3f GetSize() const;

	private:
		bool IntersectsX(AABB other) const;
		bool IntersectsY(AABB other) const;
		bool IntersectsZ(AABB other) const;

	private:
		float m_MinX;
		float m_MaxX;
		float m_MinY;
		float m_MaxY;
		float m_MinZ;
		float m_MaxZ;
	};
}