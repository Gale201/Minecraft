#include "World.h"
#include "BlockFace.h"
#include "../core/Log.h"

void World::Update()
{
	for (auto& [chunkPos, chunk] : m_Chunks)
	{
		if (chunk.mesh.needsUpdate)
		{
			BuildChunkMesh(chunk);
		}
	}
}

Vec3i World::GetChunkPosition(Vec3i pos) const
{
	return Vec3i(
		FloorDiv(pos.x, CHUNK_SIZE),
		FloorDiv(pos.y, CHUNK_HEIGHT),
		FloorDiv(pos.z, CHUNK_SIZE)
	);
}

Vec3i World::GetBlockChunkPosition(Vec3i pos) const
{
	return Vec3i(
		FloorMod(pos.x, CHUNK_SIZE),
		FloorMod(pos.y, CHUNK_HEIGHT),
		FloorMod(pos.z, CHUNK_SIZE)
	);
}

Vec3i World::GetChunkWorldPosition(Vec3i pos) const
{
	return Vec3i(
		pos.x * CHUNK_SIZE,
		pos.y * CHUNK_HEIGHT,
		pos.z * CHUNK_SIZE
	);
}

const Chunk* World::TryGetChunk(Vec3i pos) const
{
	auto it = m_Chunks.find(pos);
	if (it == m_Chunks.end())
		return nullptr;

	return &it->second;
}

Chunk& World::GetChunk(Vec3i pos)
{
	auto it = m_Chunks.find(pos);
	if (it != m_Chunks.end()) return it->second;

	Chunk chunk;
	chunk.position = pos;
	auto [newIt, _] = m_Chunks.emplace(pos, std::move(chunk));
	return newIt->second;
}

Block World::GetBlock(Vec3i pos) const
{
	Vec3i chunkPos = GetChunkPosition(pos);
	const Chunk* chunk = TryGetChunk(chunkPos);
	if (chunk == nullptr) 
		return Block(BlockType::Undefined);
	
	Vec3i blockPos = GetBlockChunkPosition(pos);
	return chunk->blocks[blockPos.x][blockPos.y][blockPos.z];
}

void World::SetBlock(Vec3i pos, BlockType type)
{
	Vec3i chunkPos = GetChunkPosition(pos);
	Chunk& chunk = GetChunk(chunkPos);

	Block block(type);
	Vec3i blockPos = GetBlockChunkPosition(pos);
	chunk.blocks[blockPos.x][blockPos.y][blockPos.z] = block;
	
	chunk.mesh.needsUpdate = true;
}

// Private functions

void World::BuildChunkMesh(Chunk& chunk)
{
	chunk.mesh.mesh.vertices.clear();
	chunk.mesh.mesh.indices.clear();

	for (int x = 0; x < CHUNK_SIZE; x++)
	{
		for (int y = 0; y < CHUNK_HEIGHT; y++)
		{
			for (int z = 0; z < CHUNK_SIZE; z++)
			{
				Block& block = chunk.blocks[x][y][z];
				if (block.type == BlockType::Air) continue;

				for (FaceDirection dir : FACE_DIRECTIONS)
				{
					Vec3i neighborPos = GetAbsoluteBlockPosition({ x, y, z }, chunk) + CUBE_NEIGHBOR_OFFSET[(int)dir];
					if (IsFaceVisible(neighborPos))
					{
						AddFaceToMesh(chunk.mesh.mesh, CUBE_FACES[(int)dir], Vec3i(x, y, z), block.type, dir);
					}
				}
			}
		}
	}

	chunk.mesh.needsUpdate = false;
}

void World::AddFaceToMesh(
	Mesh& mesh, 
	const BlockFace& face, 
	Vec3i blockPos, 
	BlockType type,
	FaceDirection dir
)
{
	int startIndex = mesh.vertices.size();

	for (int i = 0; i < 4; i++)
	{
		Vertex v;
		v.position = Vec3f(
			face.vertices[i].x + blockPos.x,
			face.vertices[i].y + blockPos.y,
			face.vertices[i].z + blockPos.z
		);
		v.normal = face.normal;

		Vec2f uvScale, uvOffset;
		GetFaceUV(type, dir, uvScale, uvOffset);
		v.u = face.uvs[i].x * uvScale.x + uvOffset.x * uvScale.x;
		v.v = face.uvs[i].y * uvScale.y + uvOffset.y * uvScale.y;

		mesh.vertices.push_back(v);
	}

	mesh.indices.push_back(startIndex + 0);
	mesh.indices.push_back(startIndex + 1);
	mesh.indices.push_back(startIndex + 2);

	mesh.indices.push_back(startIndex + 0);
	mesh.indices.push_back(startIndex + 2);
	mesh.indices.push_back(startIndex + 3);
}

bool World::IsFaceVisible(Vec3i neighborPos)
{
	Block block = GetBlock(neighborPos);
	return block.type == BlockType::Air || block.type == BlockType::Undefined;
}

Vec3i World::GetAbsoluteBlockPosition(Vec3i relPos, Chunk& chunk) const
{
	return { 
		relPos.x + chunk.position.x * CHUNK_SIZE,
		relPos.y + chunk.position.y * CHUNK_HEIGHT,
		relPos.z + chunk.position.z * CHUNK_SIZE
	};
}

inline int World::FloorDiv(int a, int b)
{
	int q = a / b;
	int r = a % b;
	if ((r != 0) && ((r < 0) != (b < 0))) q--;
	return q;
}

inline int World::FloorMod(int a, int b)
{
	int r = a % b;
	if (r < 0) r += std::abs(b);
	return r;
}

