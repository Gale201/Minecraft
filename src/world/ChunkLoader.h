#pragma once

#include <unordered_map>

#include "Chunk.h"
#include "../math/Math.h"
#include "../math/PerlinNoise.h"

namespace Minecraft
{
	class ChunkLoader
	{
	public:
		ChunkLoader(
			unsigned int seed,
			const std::unordered_map<Vec3i, ModifiedChunk, Vec3iHash>& modifiedChunks
		) : m_Seed(seed), m_ModifiedChunks(modifiedChunks) 
		{
		}
		~ChunkLoader() = default;

		void LoadChunk(Chunk& chunk) const;

		int GetSeaLevel() const { return m_SeaLevel; }
	private:
		const unsigned int m_Seed;
		const std::unordered_map<Vec3i, ModifiedChunk, Vec3iHash>& m_ModifiedChunks;

		PerlinNoise m_Perlin{ m_Seed };

		int m_SeaLevel = 60;
	};
}
