#pragma once

#include <vector>

#include "Vertex.h"

struct Mesh
{
	std::vector<Vertex> vertices;
	std::vector<int> indices;
};