#pragma once

#include <atomic>

#include "..\render\Mesh.h"
#include "Block.h"

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
	};

	struct ModifiedChunk
	{
		std::unordered_map<Vec3i, BlockType, Vec3iHash> blocks;
	};
}
