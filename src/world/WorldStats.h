#pragma once

#include <numeric>

namespace Minecraft
{
	struct WorldStats
	{
		float meshMs = 0.0f;
		float loadMs = 0.0f;
		
		uint32_t chunksMeshed = 0;
		uint32_t chunksLoaded = 0;
		uint32_t meshQueueSize = 0;
	};
}