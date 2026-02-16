#pragma once
#include "../math/Math.h"

struct WorldRaycastResult
{
	Vec3i blockPosition;
	Vec3f normal;
	float distance;
};
