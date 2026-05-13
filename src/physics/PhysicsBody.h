#pragma once

#include "..\math\Math.h"
#include "AABB.h"

namespace Minecraft
{
	struct PhysicsBody
	{
		PhysicsBody() {}
		PhysicsBody(Vec3f position) : position(position) {}
		PhysicsBody(Vec3f position, Vec3f size) : position(position), size(size) {}

		Vec3f position;
		Vec3f velocity;
		Vec3f size;
		uint32_t id = 0;
		float mass = 1.0f;
		float groundFriction = 8.0f;
		float airFriction = 1.0f;
		float flyingFriction = 0.0f;
		float waterDrag = 4.0f;
		bool applyGravity = true;
		bool collidable = true;
		bool isGrounded = false;
		bool inWater = false;
		bool pausePhysics = false;
		int collidingX = 0;
		int collidingY = 0;
		int collidingZ = 0;

		void ClearTotalForce()
		{
			m_AccumulatedForce = Vec3f();
		}

		void UpdatePreviousPosition()
		{
			m_PrevPosition = position;
		}

		void AddForce(const Vec3f& force)
		{
			m_AccumulatedForce += force;
		}

		Vec3f GetTotalForce() const
		{
			return m_AccumulatedForce;
		}

		AABB GetAABB() const
		{
			return AABB(position, size);
		}

		Vec3f GetCenter() const
		{
			return position + size * 0.5f;
		}

		Vec3f GetPreviousPosition() const
		{
			return m_PrevPosition;
		}

	private:
		Vec3f m_AccumulatedForce;
		Vec3f m_PrevPosition = position;
	};
}