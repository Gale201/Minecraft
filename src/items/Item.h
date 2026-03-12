#pragma once

#include "ItemStack.h"

#include "../math/Math.h"
#include "../physics/PhysicsBody.h"

namespace Minecraft
{
	struct Item
	{
		ItemStack stack;
		PhysicsBody body;
		Vec3f rotation;
	};
}