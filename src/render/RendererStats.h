#pragma once

#include <numeric>

namespace Minecraft
{
	struct RendererStats
	{
		float buildMs = 0.0f;
		float rasterMs = 0.0f;

		uint32_t renderCommands = 0;
		uint32_t trianglesSubmitted = 0;
		uint32_t trianglesBinned = 0;
		uint64_t pixelsTested = 0;
		uint64_t pixelsDrawn = 0;
	};
}