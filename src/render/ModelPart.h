#pragma once
#include <string>
#include "Mesh.h"

namespace Minecraft
{
	struct ModelPart
	{
		std::string name;
		Mesh mesh;
		Vec3f pivot;
	};
}