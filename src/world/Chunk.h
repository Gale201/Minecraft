#pragma once
#include "Block.h"
#include "ChunkMesh.h"
#include <atomic>

inline const int CHUNK_SIZE = 16, CHUNK_HEIGHT = 128;

enum class ChunkState
{
	Unloaded = 0,
	Generated,
	NeedsMeshing,
	Meshing,
	Ready
};

struct Chunk
{
	Vec3i position;
	Block blocks[CHUNK_SIZE][CHUNK_HEIGHT][CHUNK_SIZE];
	ChunkMesh mesh;
	ChunkMesh buildMesh;
	std::atomic<ChunkState> state = ChunkState::Unloaded;
};
