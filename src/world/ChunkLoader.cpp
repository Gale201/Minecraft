#include "ChunkLoader.h"

namespace Minecraft
{
	void ChunkLoader::LoadChunk(Chunk& chunk) const
	{
		bool chunkModified = false;
		if (m_ModifiedChunks.find(chunk.position) != m_ModifiedChunks.end())
			chunkModified = true;

		for (int x = 0; x < CHUNK_SIZE; x++)
		{
			for (int z = 0; z < CHUNK_SIZE; z++)
			{
				float worldX = static_cast<float>(chunk.position.x * CHUNK_SIZE + x);
				float worldZ = static_cast<float>(chunk.position.z * CHUNK_SIZE + z);

				float seaLevel = 50.0f;

				float base = m_Perlin.fbm(worldX * 0.01f, worldZ * 0.01f, 3, 0.5f);
				float continent = m_Perlin.fbm(worldX * 0.001f, worldZ * 0.001f, 2, 0.5f) - m_Perlin.fbm(worldX * 0.001f, worldZ * 0.001f, 4, 0.5f) * 0.1f;
				float mountainMask = m_Perlin.fbm(worldX * 0.001f, worldZ * 0.001f, 2, 0.5f);
				mountainMask = std::fmaxf(0, mountainMask - 0.7f) * 3.5f;
				float mountainHeight = m_Perlin.fbm(worldX * 0.01f, worldZ * 0.01f, 4, 0.5f);
				mountainHeight = 1.0f - std::abs(mountainHeight) + m_Perlin.fbm(worldX * 0.01f, worldZ * 0.01f, 6, 0.5f) * 4.0f;
				float mountains = mountainHeight * mountainMask;

				float h = base * 50.0f * continent + continent * 70.0f + mountains * 80.0f;
				int height = static_cast<int>(std::fmaxf(std::fminf(h, CHUNK_HEIGHT), 0));

				for (int y = 0; y < CHUNK_HEIGHT; y++)
				{
					Vec3i blockPos = Vec3i(x, y, z);
					if (chunkModified && m_ModifiedChunks.at(chunk.position).blocks.find(blockPos) != m_ModifiedChunks.at(chunk.position).blocks.end())
					{
						chunk.blocks[x][y][z] = Block(m_ModifiedChunks.at(chunk.position).blocks.at(blockPos));
						continue;
					}
					if (y >= height) continue;

					if (height < seaLevel && y == height - 1) chunk.blocks[x][y][z] = { BlockType::Dirt };
					else if (y == height - 1) chunk.blocks[x][y][z] = { BlockType::Grass };
					else if (y > height - 6) chunk.blocks[x][y][z] = { BlockType::Dirt };
					else chunk.blocks[x][y][z] = Block(BlockType::Stone);
				}
			}
		}
	}
}
