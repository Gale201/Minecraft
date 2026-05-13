#pragma once

#include "ItemStack.h"

#include "../math/Math.h"
#include "../physics/PhysicsBody.h"

#define ITEM_TEXTURE_SIZE 100

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
		float pickupCooldown = 0.0f;
	};
}