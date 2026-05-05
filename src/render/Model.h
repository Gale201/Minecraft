#pragma once
#include <vector>
#include "Mesh.h"
#include "ModelPart.h"

namespace Minecraft
{
	struct Model
	{
		std::vector<ModelPart> parts;
	};
}
