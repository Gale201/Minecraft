#pragma once

#include <vector>

#include "../render/Vertex.h"
#include "../render/Mesh.h"

struct ChunkMesh
{
	Mesh mesh;
	bool needsUpdate = true;
};
