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

	Mesh CreateBlockMesh(BlockType blockType, float scale, Vec3f centerOffset);
	Mesh CreateBlockMesh(AtlasTileInfo tileInfo, float scale, Vec3f centerOffset);
}
