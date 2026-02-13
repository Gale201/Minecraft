#pragma once
#include "Block.h"
#include "ChunkMesh.h"

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
	ChunkState state = ChunkState::Unloaded;
};
