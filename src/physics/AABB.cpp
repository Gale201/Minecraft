#include "AABB.h"

namespace Minecraft
{
	AABB AABB::Expand(Vec3f value) const
	{
		AABB aabb(*this);
		if (value.x > 0.0f) aabb.m_MaxX += value.x;
		else aabb.m_MinX += value.x;

		if (value.y > 0.0f) aabb.m_MaxY += value.y;
		else aabb.m_MinY += value.y;

		if (value.z > 0.0f) aabb.m_MaxZ += value.z;
		else aabb.m_MinZ += value.z;

		return aabb;
	}

	AABB AABB::Grow(Vec3f value) const
	{
		AABB aabb(*this);

		aabb.m_MinX -= value.x;
		aabb.m_MinY -= value.y;
		aabb.m_MinZ -= value.z;

		aabb.m_MinX += value.x;
		aabb.m_MinY += value.y;
		aabb.m_MinZ += value.z;

		return aabb;
	}

	void AABB::Move(Vec3f value)
	{
		m_MinX += value.x;
		m_MinY += value.y;
		m_MinZ += value.z;
		m_MaxX += value.x;
		m_MaxY += value.y;
		m_MaxZ += value.z;
	}

	bool AABB::Intersects(AABB other) const
	{
		return IntersectsX(other) && IntersectsY(other) && IntersectsZ(other);
	}

	float AABB::GetClipX(AABB other, float delta) const
	{
		if (!IntersectsY(other) || !IntersectsZ(other)) return delta;

		if (delta > 0 && m_MaxX <= other.m_MinX)
		{
			float clip = other.m_MinX - m_MaxX;
			if (delta > clip) delta = clip;
		}
		if (delta < 0 && m_MinX >= other.m_MinX)
		{
			float clip = other.m_MaxX - m_MinX;
			if (delta < clip) delta = clip;
		}
		return delta;
	}

	float AABB::GetClipY(AABB other, float delta) const
	{
		if (!IntersectsX(other) || !IntersectsZ(other)) return delta;

		if (delta > 0 && m_MaxY <= other.m_MinY)
		{
			float clip = other.m_MinY - m_MaxY;
			if (delta > clip) delta = clip;
		}
		if (delta < 0 && m_MinY >= other.m_MinY)
		{
			float clip = other.m_MaxY - m_MinY;
			if (delta < clip) delta = clip;
		}
		return delta;
	}

	float AABB::GetClipZ(AABB other, float delta) const
	{
		if (!IntersectsX(other) || !IntersectsY(other)) return delta;

		if (delta > 0 && m_MaxZ <= other.m_MinZ)
		{
			float clip = other.m_MinZ - m_MaxZ;
			if (delta > clip) delta = clip;
		}
		if (delta < 0 && m_MinZ >= other.m_MinZ)
		{
			float clip = other.m_MaxZ - m_MinZ;
			if (delta < clip) delta = clip;
		}
		return delta;
	}

	Vec3f AABB::GetPosition() const
	{
		return Vec3f(m_MinX, m_MinY, m_MinZ);
	}

	Vec3f AABB::GetSize() const
	{
		return Vec3f(m_MaxX - m_MinX, m_MaxY - m_MinY, m_MaxZ - m_MinZ);
	}

	/* Private functions */

	bool AABB::IntersectsX(AABB other) const
	{
		return m_MinX < other.m_MaxX && m_MaxX > other.m_MinX; 
	}

	bool AABB::IntersectsY(AABB other) const
	{
		return m_MinY < other.m_MaxY && m_MaxY > other.m_MinY;
	}

	bool AABB::IntersectsZ(AABB other) const
	{
		return m_MinZ < other.m_MaxZ && m_MaxZ > other.m_MinZ;
	}
}