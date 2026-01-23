#pragma once
#include "Block.h"
#include "ChunkMesh.h"

inline const int CHUNK_SIZE = 16, CHUNK_HEIGHT = 16;

struct Chunk
{
	Vec3i position;
	Block blocks[CHUNK_HEIGHT][CHUNK_SIZE][CHUNK_SIZE];
	ChunkMesh mesh;
};
