#pragma once

#include <vector>

#include "Vertex.h"

namespace Minecraft
{
	struct Mesh
	{
		std::vector<Vertex> vertices;
		std::vector<int> indices;
		bool shade = true;
	};
}
