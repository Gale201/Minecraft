#pragma once
#include "../math/Math.h"

namespace Minecraft
{
	struct WorldRaycastResult
	{
		Vec3i blockPosition;
		Vec3f normal;
		float distance;
	};
}
