#pragma once
#include "../math/Math.h"

struct Plane
{
	Vec3f normal;
	float d;
};

struct Frustum
{
	Plane planes[6];
};
