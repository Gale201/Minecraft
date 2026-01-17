#pragma once

#include "Model.h"
#include "Transform.h"

struct RenderObject
{
	Model model;
	Transform transform;
	uint32_t color;
};
