#include "ChunkLoader.h"

#include <random>

#include "../timer/Timer.h"

namespace Minecraft
{
	void ChunkLoader::LoadChunk(Chunk& chunk) const
	{
		const ModifiedChunk* modifiedChunk = nullptr;
		auto it = m_ModifiedChunks.find(chunk.position);
		if (it != m_ModifiedChunks.end()) modifiedChunk = &it->second;

		std::mt19937 rng(m_Seed + chunk.position.x * 341873128712 + chunk.position.z * 132897987541);

		for (int x = 0; x < CHUNK_SIZE; x++)
		{
			for (int z = 0; z < CHUNK_SIZE; z++)
			{
				float worldX = static_cast<float>(chunk.position.x * CHUNK_SIZE + x);
				float worldZ = static_cast<float>(chunk.position.z * CHUNK_SIZE + z);

				float base = m_Perlin.fbm(worldX * 0.01f, worldZ * 0.01f, 3, 0.5f);
				float continent = m_Perlin.fbm(worldX * 0.005f, worldZ * 0.005f, 2, 0.5f) - m_Perlin.fbm(worldX * 0.005f + 0.04f, worldZ * 0.005f + 0.04f, 4, 0.5f) * 0.1f;
				float mountainMask = m_Perlin.fbm(worldX * 0.001f, worldZ * 0.001f, 2, 0.5f);
				mountainMask = std::fmaxf(0, mountainMask - 0.7f) * 3.5f;
				float mountainHeight = m_Perlin.fbm(worldX * 0.01f, worldZ * 0.01f, 4, 0.5f);
				mountainHeight = 1.0f - std::abs(mountainHeight) + m_Perlin.fbm(worldX * 0.01f, worldZ * 0.01f, 6, 0.5f) * 4.0f;
				float mountains = mountainHeight * mountainMask;

				float h = base * 50.0f * continent + continent * 70.0f + mountains * 50.0f;
				int height = static_cast<int>(std::fmaxf(std::fminf(h, CHUNK_HEIGHT), 0));

				for (int y = 0; y < CHUNK_HEIGHT; y++)
				{
					Vec3i blockPos = Vec3i(x, y, z);
					if (y >= height && y <= m_SeaLevel) chunk.blocks[x][y][z].type = BlockType::Water;
					if (y >= height) continue;

					if (height <= m_SeaLevel && y == height - 1) chunk.blocks[x][y][z].type = BlockType::Dirt;
					else if (y == height - 1) chunk.blocks[x][y][z].type = BlockType::Grass;
					else if (y > height - 6) chunk.blocks[x][y][z].type = BlockType::Dirt;
					else chunk.blocks[x][y][z].type = BlockType::Stone;
				}

				// Gravel
				float gravelBase = m_Perlin.fbm(worldX * 0.01f, worldZ * 0.01f, 1, 0.5f);
				if (height < m_SeaLevel - 5)
				{
					if (gravelBase + base * continent + continent < 1.2f) chunk.blocks[x][height][z].type = BlockType::Gravel;
				}

				// Sand
				float sandBase = m_Perlin.fbm(worldX * 0.05f, worldZ * 0.05f, 5, 0.5f);
				if (height >= m_SeaLevel - 2 && height <= m_SeaLevel + 2 && (
					chunk.blocks[x][height - 1][z].type == BlockType::Grass ||
					chunk.blocks[x][height - 1][z].type == BlockType::Dirt
					))
				{
					int aroundWater = 0;
					for (int cy = -1; cy <= 1; cy++)
						for (int cx = -2; cx <= 2; cx++)
							for (int cz = -2; cz <= 2; cz++)
							{
								if (x + cx < 0 || x + cx >= CHUNK_SIZE || z + cz < 0 || x + cz >= CHUNK_SIZE)
									continue;
								if (chunk.blocks[x + cx][height - 1 + cy][z + cz].type == BlockType::Water)
									aroundWater += 1;
							}
					if (aroundWater > 0 && sandBase > 0.4f / aroundWater)
						chunk.blocks[x][height - 1][z].type = BlockType::Sand;
				}

				// Bedrock
				std::uniform_int_distribution<int> bedrockDist(0, 2);
				int bedrockBase = bedrockDist(rng);
				for (int y = 0; y < 4; y++)
				{
					if (y == 0) chunk.blocks[x][y][z].type = BlockType::Bedrock;
					else if (y < bedrockBase + 1)
						chunk.blocks[x][y][z].type = BlockType::Bedrock;
				}

				// Flowers
				std::uniform_int_distribution<int> flowerDist(0, 127);
				if (height > m_SeaLevel && flowerDist(rng) == 0 && height < CHUNK_HEIGHT && chunk.blocks[x][height - 1][z].type == BlockType::Grass)
					chunk.blocks[x][height][z].type = BlockType::Rose;
				else if (height > m_SeaLevel && flowerDist(rng) == 0 && height < CHUNK_HEIGHT && chunk.blocks[x][height - 1][z].type == BlockType::Grass)
					chunk.blocks[x][height][z].type = BlockType::YellowFlower;
			
				// Trees
				float treeBase = m_Perlin.fbm(worldX * 0.001f, worldZ * 0.001f, 6, 0.5f);
				std::uniform_int_distribution<int> treeDist(0, 49);
				std::uniform_int_distribution<int> treeDist2(0, 2);
				if (height > m_SeaLevel && treeBase > 0.3f && treeDist(rng) == 0 && height < CHUNK_HEIGHT && x > 3 && x < CHUNK_SIZE - 3 && z > 3 && z < CHUNK_SIZE - 3 && chunk.blocks[x][height - 1][z].type == BlockType::Grass)
				{
					int th = 0;
					for (int ty = height; ty < CHUNK_HEIGHT - 3; ty++)
					{
						th++;
						if (th > 5) break;
						if (th > 3 && treeDist2(rng) == 0) break;
						chunk.blocks[x][ty][z].type = BlockType::OakLog;
					}

					int ly = th * 2 / 3;
					for (int cy = ly; cy < th + 2; cy++)
					{
						int r = 2;
						if (cy >= th) r--;
						if (cy >= th + 1) r--;
						for (int cx = -r; cx <= r; cx++)
						{
							for (int cz = -r; cz <= r; cz++)
							{
								if (x + cx < 0 || x + cx >= CHUNK_SIZE || z + cz < 0 || z + cz >= CHUNK_SIZE) continue;
								if (chunk.blocks[x + cx][height + cy][z + cz].type == BlockType::Air) chunk.blocks[x + cx][height + cy][z + cz].type = BlockType::OakLeaves;
							}
						}
					}
				}
			}
		}

		if (modifiedChunk)
		{
			for (const auto& [blockPos, block] : modifiedChunk->blocks)
			{
				chunk.blocks[blockPos.x][blockPos.y][blockPos.z] = block;
			}
		}
	}
}
