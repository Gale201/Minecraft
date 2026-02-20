#pragma once
#include "../math/Math.h"

namespace Minecraft
{
	struct Plane
	{
		Vec3f normal;
		float d;
	};

	struct Frustum
	{
		Plane planes[6];
	};
}
