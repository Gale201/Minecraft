#pragma once
#include <vector>
#include "Mesh.h"

namespace Minecraft
{
	struct Model
	{
		std::vector<std::string> meshNames;
		std::vector<Mesh> meshes;
	};
}
