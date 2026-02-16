#pragma once

#include "Chunk.h"
#include "../math/Math.h"
#include "../math/PerlinNoise.h"

class ChunkLoader
{
public:
	ChunkLoader(uint64_t seed) : m_Seed(seed) {}
	~ChunkLoader() = default;

	void LoadChunk(Chunk& chunk) const;
private:
	const uint64_t m_Seed;
	PerlinNoise m_Perlin{ m_Seed };
};