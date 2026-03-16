#pragma once

#include "ItemStack.h"

#include "../math/Math.h"
#include "../physics/PhysicsBody.h"

namespace Minecraft
{
	using ItemID = uint32_t;

	struct Item
	{
		ItemID id;
		ItemStack stack;
		PhysicsBody* body;
		Vec3f rotation;
		Vec3i chunkPos;
	};
}