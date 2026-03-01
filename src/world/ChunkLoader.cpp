#include "ChunkLoader.h"
#include "../timer/Timer.h"

namespace Minecraft
{
	void ChunkLoader::LoadChunk(Chunk& chunk) const
	{
		const ModifiedChunk* modifiedChunk = nullptr;
		auto it = m_ModifiedChunks.find(chunk.position);
		if (it != m_ModifiedChunks.end()) modifiedChunk = &it->second;

		for (int x = 0; x < CHUNK_SIZE; x++)
		{
			for (int z = 0; z < CHUNK_SIZE; z++)
			{
				float worldX = static_cast<float>(chunk.position.x * CHUNK_SIZE + x);
				float worldZ = static_cast<float>(chunk.position.z * CHUNK_SIZE + z);

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
					if (y >= height && y < m_SeaLevel) chunk.blocks[x][y][z] = { BlockType::Water };
					if (y >= height) continue;

					if (height < m_SeaLevel && y == height - 1) chunk.blocks[x][y][z] = { BlockType::Dirt };
					else if (y == height - 1) chunk.blocks[x][y][z] = { BlockType::Grass };
					else if (y > height - 6) chunk.blocks[x][y][z] = { BlockType::Dirt };
					else chunk.blocks[x][y][z] = { BlockType::Stone };
				}

				// Bedrock
				int bedrockBase = rand() % 3;
				for (int y = 0; y < 4; y++)
				{
					if (y == 0) chunk.blocks[x][y][z] = { BlockType::Bedrock };
					else if (y < bedrockBase + 1)
						chunk.blocks[x][y][z] = { BlockType::Bedrock };
				}

				// Flowers
				if (height > m_SeaLevel && rand() % 128 == 0 && height < CHUNK_HEIGHT) chunk.blocks[x][height][z] = { BlockType::Rose };
				else if (height > m_SeaLevel && rand() % 128 == 0 && height < CHUNK_HEIGHT) chunk.blocks[x][height][z] = { BlockType::YellowFlower };
			}
		}

		if (modifiedChunk)
		{
			for (const auto& [blockPos, blockType] : modifiedChunk->blocks)
			{
				chunk.blocks[blockPos.x][blockPos.y][blockPos.z] = { blockType };
			}
		}
	}
}
