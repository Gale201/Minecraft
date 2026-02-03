#pragma once
#include "Mesh.h"
#include "Transform.h"
#include "Texture.h"

struct RenderCommand
{
	const Mesh* mesh;
	Transform transform;
	const Texture* texture;
};
