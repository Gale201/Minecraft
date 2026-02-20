#pragma once

#include <stdexcept>

#include "Vertex.h"

namespace Minecraft
{
	struct Triangle
	{
		Vertex v0;
		Vertex v1;
		Vertex v2;
		float lightIntensity = 0.0f;

		void SetVertexByIndex(size_t index, const Vertex& v)
		{
			switch (index)
			{
			case 0:
				v0 = v;
				break;
			case 1:
				v1 = v;
				break;
			case 2:
				v2 = v;
				break;
			default:
				throw std::out_of_range("Triangle vertex index out of range!");
			}
		}

		Vertex GetVertexByIndex(size_t index) const
		{
			switch (index)
			{
			case 0:
				return v0;
			case 1:
				return v1;
			case 2:
				return v2;
			default:
				throw std::out_of_range("Triangle vertex index out of range!");
			}
		}
	};
}
