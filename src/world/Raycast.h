#pragma once
#include "../math/Math.h"

namespace Minecraft
{
	struct WorldRaycastResult
	{
		Vec3i blockPosition;
		Vec3f normal;
		BlockType blockType{ BlockType::Undefined };
		float distance{ 0.0f };
	};

	inline bool RaycastAgainstBox(const Vec3f& origin, const Vec3f direction, const Vec3f& min, const Vec3f& max, float& out)
	{
        float t1 = (min.x - origin.x) / direction.x;
        float t2 = (max.x - origin.x) / direction.x;
        float t3 = (min.y - origin.y) / direction.y;
        float t4 = (max.y - origin.y) / direction.y;
        float t5 = (min.z - origin.z) / direction.z;
        float t6 = (max.z - origin.z) / direction.z;

        float tmin = std::max(
            std::max(std::min(t1, t2), std::min(t3, t4)),
            std::min(t5, t6)
        );

        float tmax = std::min(
            std::min(std::max(t1, t2), std::max(t3, t4)),
            std::max(t5, t6)
        );

        if (tmax < 0) return false;
        if (tmin > tmax) return false;

        out = tmin;
        return true;
	}
}
