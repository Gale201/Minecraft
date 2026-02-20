#pragma once
#include "../math/Math.h"
#include "../world/BlockFace.h"

namespace Minecraft
{
	struct Vertex
	{
		Vec3f position;
		Vec3f normal;
		float u, v;
		float invW;
		AtlasTileInfo tileInfo;
	};
}
