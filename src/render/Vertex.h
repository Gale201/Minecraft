#pragma once
#include "../math/Math.h"

struct Vertex
{
	Vec3f position;
	Vec3f normal;
	float u, v;
	float invW;
};