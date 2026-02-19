#pragma once

#include "Chunk.h"
#include "../math/Math.h"
#include "../math/PerlinNoise.h"

class ChunkLoader
{
public:
	ChunkLoader(unsigned int seed) : m_Seed(seed) {}
	~ChunkLoader() = default;

	void LoadChunk(Chunk& chunk) const;
private:
	const unsigned int m_Seed;
	PerlinNoise m_Perlin{ m_Seed };
};