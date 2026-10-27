#pragma once

#include <atomic>

#include "..\render\Mesh.h"
#include "Block.h"
#include <assert.h>

namespace Minecraft
{
	inline const int CHUNK_SIZE = 16, CHUNK_HEIGHT = 128;

	enum class ChunkState
	{
		Unloaded = 0,
		Generated,
		NeedsMeshing,
		Meshing,
		Ready
	};

	struct ChunkMesh
	{
		Mesh meshes[6];
	};

	struct Chunk
	{
		Vec3i position;
		Block blocks[CHUNK_SIZE][CHUNK_HEIGHT][CHUNK_SIZE];
		ChunkMesh mesh;
		ChunkMesh buildMesh;
		std::atomic<ChunkState> state = ChunkState::Unloaded;

		static constexpr bool InBounds(int x, int y, int z)
		{
			return x >= 0 && x < CHUNK_SIZE && y >= 0 && y < CHUNK_HEIGHT && z >= 0 && z < CHUNK_SIZE;
		}
		
		Block& At(int x, int y, int z)
		{
			assert(InBounds(x, y, z));
			return blocks[x][y][z];
		}
	};

	struct ModifiedChunk
	{
		std::unordered_map<Vec3i, Block, Vec3iHash> blocks;
	};
}
