#pragma once

#include "Model.h"
#include "Transform.h"
#include "Texture.h"

namespace Minecraft
{
	struct RenderObject
	{
		Model& model;
		Transform transform;
		Texture& texture;
	};
}
