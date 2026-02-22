#pragma once

#include "..\math\Math.h"
#include "AABB.h"

namespace Minecraft
{
	struct PhysicsBody
	{
		Vec3f position;
		Vec3f velocity;
		Vec3f size;
		bool applyGravity = true;
		bool isGrounded = false;

		AABB GetAABB() const
		{
			return AABB(position, size);
		}
	};
}