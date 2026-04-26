#pragma once
#include "../math/Math.h"
#include "../render/Mesh.h"

namespace Minecraft
{
	struct EntityModelPart
	{
		Vec3f position;
		Vec3f scale{ 1.0f, 1.0f, 1.0f };
		Vec3f rotation;
		const Mesh* mesh;
	};
}
