#include "ChunkLoader.h"

Chunk ChunkLoader::LoadChunk(Vec3i chunkPosition)
{
	Chunk chunk;
	chunk.position = chunkPosition;

	for (int j = 0; j < 11; j++)
	{
		for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
		{
			chunk.blocks[i / 16][j + 4][i % 16] = { BlockType::Stone };
		}
	}
	for (int j = 0; j < 4; j++)
	{
		for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
		{
			chunk.blocks[i / 16][j + 11][i % 16] = { BlockType::Dirt };
		}
	}

	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++)
	{
		chunk.blocks[i / 16][15][i % 16] = { BlockType::Grass };
	}

	return chunk;
}