#pragma once

#include <vector>

#include "../render/Vertex.h"
#include "../render/Mesh.h"

struct ChunkMesh
{
	Mesh meshes[6];
	bool needsUpdate = true;
};
