#pragma once

#include "Chunk.h"
#include "../math/Math.h"

class ChunkLoader
{
public:
	ChunkLoader(uint64_t seed) : m_Seed(seed) {}
	~ChunkLoader() = default;

	Chunk LoadChunk(Vec3i chunkPosition);
private:
	uint64_t m_Seed = 0;
};