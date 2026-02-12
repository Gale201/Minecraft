#include "World.h"
#include "BlockFace.h"
#include "../core/Log.h"
#include "../render/Frustum.h"
#include "../render/Camera.h"

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

void World::LoadAndUnloadChunks(Vec3f playerPosition)
{
	LoadChunks(playerPosition);
	UnloadChunks(playerPosition);
}

void World::BuildRenderList(const Camera& camera, std::vector<RenderCommand>& outCommands)
{
	const std::vector<const Chunk*> visibleChunks = GetFrustumVisibleChunks(camera);
	for (const Chunk* chunk : visibleChunks)
	{
		Transform t;
		t.SetPosition(GetChunkWorldPosition(chunk->position).toVec3f());

		for (int dir = 0; dir < 6; dir++)
		{
			outCommands.push_back({
				&chunk->mesh.meshes[dir],
				t,
				&m_TextureAtlas
			});
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

const std::vector<const Chunk*> World::GetFrustumVisibleChunks(const Camera& camera) const
{
	std::vector<const Chunk*> visible;
	visible.reserve(m_Chunks.size());

	const Frustum& frustum = camera.GetFrustum();

	for (const auto& [chunkPos, chunk] : m_Chunks)
	{
		if (FrustumTestChunk(chunkPos, frustum))
		{
			visible.push_back(&chunk);
		}
	}
	return visible;
}

/* 
 Private functions 
*/

/* Chunk loading */

void World::LoadChunks(Vec3f playerPosition)
{
	if (playerPosition == m_LastChunkLoadPosition) return;

	int playerX = FloorDiv(static_cast<int>(std::floor(playerPosition.x)), CHUNK_SIZE);
	int playerZ = FloorDiv(static_cast<int>(std::floor(playerPosition.z)), CHUNK_SIZE);


	// WRONG!! Just check for x,-x,z,-z if the player moved in that dir, and then load the 32 chunk row in that dir
	for (int x = playerX - m_ChunkLoadRadius; x <= playerX + m_ChunkLoadRadius; x++) 
	{ 
		for (int z = playerZ - m_ChunkLoadRadius; z <= playerZ + m_ChunkLoadRadius; z++)
		{
			Vec3i chunkPos = Vec3i(x, 0, z);
			if (m_Chunks.find(chunkPos) != m_Chunks.end())  // If the chunk is already loaded, skip it
				continue;

			// Load the chunk and add it to the world
			Chunk chunk = m_ChunkLoader.LoadChunk(chunkPos);
			m_Chunks.emplace(chunkPos, std::move(chunk));
		}
	}
}

void World::UnloadChunks(Vec3f playerPosition)
{
	if (playerPosition == m_LastChunkLoadPosition) return;

	int playerX = FloorDiv(static_cast<int>(std::floor(playerPosition.x)), CHUNK_SIZE);
	int playerZ = FloorDiv(static_cast<int>(std::floor(playerPosition.z)), CHUNK_SIZE);

	std::vector<Vec3i> chunksToUnload; 
	for (const auto& [chunkPos, chunk] : m_Chunks)
	{
		if (chunkPos.x < playerX - m_ChunkLoadRadius || chunkPos.x > playerX + m_ChunkLoadRadius ||
			chunkPos.z < playerZ - m_ChunkLoadRadius || chunkPos.z > playerZ + m_ChunkLoadRadius) 
		{
			chunksToUnload.push_back(chunkPos); 
		} 
	}
	for (const Vec3i& pos : chunksToUnload)
	{ 
		m_Chunks.erase(pos);
	}

	m_LastChunkLoadPosition = playerPosition;
}

/* Chunk mesh building */

void World::BuildChunkMesh(Chunk& chunk)
{
	for (Mesh& meshPart : chunk.mesh.meshes)
	{
		meshPart.vertices.clear();
		meshPart.indices.clear();
	}

	Mat3D<Block> gridPosX = { CHUNK_SIZE, CHUNK_HEIGHT, CHUNK_SIZE };
	Mat3D<Block> gridNegX = { CHUNK_SIZE, CHUNK_SIZE, CHUNK_HEIGHT };
	Mat3D<Block> gridPosY = { CHUNK_HEIGHT, CHUNK_SIZE, CHUNK_SIZE };
	Mat3D<Block> gridNegY = { CHUNK_HEIGHT, CHUNK_SIZE, CHUNK_SIZE };
	Mat3D<Block> gridPosZ = { CHUNK_SIZE, CHUNK_HEIGHT, CHUNK_SIZE };
	Mat3D<Block> gridNegZ = { CHUNK_SIZE, CHUNK_SIZE, CHUNK_HEIGHT };


	for (int x = 0; x < CHUNK_SIZE; x++)
	{
		for (int y = 0; y < CHUNK_HEIGHT; y++)
		{
			for (int z = 0; z < CHUNK_SIZE; z++)
			{
				Block& block = chunk.blocks[x][y][z];
				if (block.type == BlockType::Undefined || block.type == BlockType::Air) continue;

				for (FaceDirection dir : FACE_DIRECTIONS)
				{
					Vec3i neighborPos = GetAbsoluteBlockPosition({ x, y, z }, chunk) + CUBE_NEIGHBOR_OFFSET[(int)dir];
					if (IsFaceVisible(neighborPos))
					{
						// AddFaceToMesh(chunk.mesh.mesh, CUBE_FACES[(int)dir], Vec3i(x, y, z), block.type, dir);
						
						if (dir == FaceDirection::PosY)
							gridPosY(y, x, z).type = block.type;
						else if (dir == FaceDirection::PosX)
							gridPosX(x, y, z).type = block.type;
						else if (dir == FaceDirection::PosZ)
							gridPosZ(z, y, x).type = block.type;
						else if (dir == FaceDirection::NegX)
							gridNegX(x, z, y).type = block.type;
						else if (dir == FaceDirection::NegY)
							gridNegY(y, z, x).type = block.type;
						else if (dir == FaceDirection::NegZ)
							gridNegZ(z, x, y).type = block.type;
					}
					else
					{
						if (dir == FaceDirection::PosY)
							gridPosY(y, x, z).type = BlockType::Undefined;
						else if (dir == FaceDirection::PosX)
							gridPosX(x, y, z).type = BlockType::Undefined;
						else if (dir == FaceDirection::PosZ)
							gridPosZ(z, y, x).type = BlockType::Undefined;
						else if (dir == FaceDirection::NegX)
							gridNegX(x, z, y).type = BlockType::Undefined;
						else if (dir == FaceDirection::NegY)
							gridNegY(y, z, x).type = BlockType::Undefined;
						else if (dir == FaceDirection::NegZ)
							gridNegZ(z, x, y).type = BlockType::Undefined;
					}
				}
			}
		}
	}

	MergePosX(gridPosX, chunk.mesh.meshes[0]);
	MergeNegX(gridNegX, chunk.mesh.meshes[1]);
	MergePosY(gridPosY, chunk.mesh.meshes[2]);
	MergeNegY(gridNegY, chunk.mesh.meshes[3]);
	MergePosZ(gridPosZ, chunk.mesh.meshes[4]);
	MergeNegZ(gridNegZ, chunk.mesh.meshes[5]);

	chunk.mesh.needsUpdate = false;
}

void World::MergePosX(Mat3D<Block>& grid, Mesh& mesh)
{
	int x, y, z;
	for (x = 0; x < CHUNK_SIZE; x++)
	{
		for (y = 0; y < CHUNK_HEIGHT; y++)
		{
			for (z = 0; z < CHUNK_SIZE; z++)
			{
				BlockFace face = CUBE_FACES[(int)FaceDirection::PosX];
				BlockType type = grid(x, y, z).type;

				if (type == BlockType::Air || type == BlockType::Undefined)
					continue;

				Vec3f topLeft = face.vertices[0] + Vec3f(x, y, z);
				Vec3f topRight;
				Vec3f bottomRight;
				Vec3f bottomLeft;

				int endZ, endY;
				for (endY = y + 1; endY < CHUNK_HEIGHT && grid(x, endY, z).type == type; endY++)
				{
					grid(x, endY, z).type = BlockType::Undefined;
				}

				for (endZ = z + 1; endZ < CHUNK_SIZE; endZ++)
				{
					bool passed = true;
					for (int cy = y; cy < endY; cy++)
					{
						if (grid(x, cy, endZ).type != type)
						{
							passed = false;
							break;
						}
					}
					if (passed)
					{
						for (int cy = y; cy < endY; cy++)
							grid(x, cy, endZ).type = BlockType::Undefined;
					}
					else
						break;
				}

				topRight = face.vertices[1] + Vec3f(x, endY - 1, z);
				bottomRight = face.vertices[2] + Vec3f(x, endY - 1, endZ - 1);
				bottomLeft = face.vertices[3] + Vec3f(x, y, endZ - 1);

				BlockFace mergedFace;
				mergedFace.vertices[0] = topLeft;
				mergedFace.vertices[1] = topRight;
				mergedFace.vertices[2] = bottomRight;
				mergedFace.vertices[3] = bottomLeft;
				mergedFace.normal = face.normal;
				for (int i = 0; i < 4; i++)
					mergedFace.uvs[i] = face.uvs[i];

				AddMergedFaceToMesh(mesh, mergedFace, type, FaceDirection::PosX, endZ - z, endY - y);

				z = endZ - 1;
			}
		}
	}
}

void World::MergePosY(Mat3D<Block>& grid, Mesh& mesh)
{
	int x, y, z;
	for (y = 0; y < CHUNK_HEIGHT; y++)
	{
		for (x = 0; x < CHUNK_SIZE; x++)
		{
			for (z = 0; z < CHUNK_SIZE; z++)
			{
				BlockFace face = CUBE_FACES[(int)FaceDirection::PosY];
				BlockType type = grid(y, x, z).type;

				if (type == BlockType::Air || type == BlockType::Undefined)
					continue;

				Vec3f topLeft = face.vertices[0] + Vec3f(x, y, z);
				Vec3f topRight;
				Vec3f bottomRight;
				Vec3f bottomLeft;

				int endX, endZ;
				for (endZ = z + 1; endZ < CHUNK_SIZE && grid(y, x, endZ).type == type; endZ++)
				{
					grid(y, x, endZ).type = BlockType::Undefined;
				}

				for (endX = x + 1; endX < CHUNK_SIZE; endX++)
				{
					bool passed = true;
					for (int cz = z; cz < endZ; cz++)
					{
						if (grid(y, endX, cz).type != type)
						{
							passed = false;
							break;
						}
					}
					if (passed)
					{
						for (int cz = z; cz < endZ; cz++)
							grid(y, endX, cz).type = BlockType::Undefined;
					}
					else
						break;
				}

				topRight = face.vertices[1] + Vec3f(x, y, endZ - 1);
				bottomRight = face.vertices[2] + Vec3f(endX - 1, y, endZ - 1);
				bottomLeft = face.vertices[3] + Vec3f(endX - 1, y, z);

				BlockFace mergedFace;
				mergedFace.vertices[0] = topLeft;
				mergedFace.vertices[1] = topRight;
				mergedFace.vertices[2] = bottomRight;
				mergedFace.vertices[3] = bottomLeft;
				mergedFace.normal = face.normal;
				for (int i = 0; i < 4; i++)
					mergedFace.uvs[i] = face.uvs[i];

				AddMergedFaceToMesh(mesh, mergedFace, type, FaceDirection::PosY, endX - x, endZ - z);

				z = endZ - 1;
			}
		}
	}
}

void World::MergePosZ(Mat3D<Block>& grid, Mesh& mesh)
{
	int x, y, z;
	for (z = 0; z < CHUNK_SIZE; z++)
	{
		for (y = 0; y < CHUNK_HEIGHT; y++)
		{
			for (x = 0; x < CHUNK_SIZE; x++)
			{
				BlockFace face = CUBE_FACES[(int)FaceDirection::PosZ];
				BlockType type = grid(z, y, x).type;

				if (type == BlockType::Air || type == BlockType::Undefined)
					continue;

				Vec3f topLeft = face.vertices[3] + Vec3f(x, y, z);
				Vec3f topRight;
				Vec3f bottomRight;
				Vec3f bottomLeft;

				int endY, endX;
				for (endX = x + 1; endX < CHUNK_SIZE && grid(z, y, endX).type == type; endX++)
				{
					grid(z, y, endX).type = BlockType::Undefined;
				}

				for (endY = y + 1; endY < CHUNK_HEIGHT; endY++)
				{
					bool passed = true;
					for (int cx = x; cx < endX; cx++)
					{
						if (grid(z, endY, cx).type != type)
						{
							passed = false;
							break;
						}
					}
					if (passed)
					{
						for (int cx = x; cx < endX; cx++)
							grid(z, endY, cx).type = BlockType::Undefined;
					}
					else
						break;
				}

				topRight = face.vertices[2] + Vec3f(x, endY - 1, z);
				bottomRight = face.vertices[1] + Vec3f(endX - 1, endY - 1, z);
				bottomLeft = face.vertices[0] + Vec3f(endX - 1, y, z);

				BlockFace mergedFace;
				mergedFace.vertices[3] = topLeft;
				mergedFace.vertices[2] = topRight;
				mergedFace.vertices[1] = bottomRight;
				mergedFace.vertices[0] = bottomLeft;
				mergedFace.normal = face.normal;
				for (int i = 0; i < 4; i++)
					mergedFace.uvs[i] = face.uvs[i];

				AddMergedFaceToMesh(mesh, mergedFace, type, FaceDirection::PosZ, endX - x, endY - y);

				x = endX - 1;
			}
		}
	}
}

void World::MergeNegX(Mat3D<Block>& grid, Mesh& mesh)
{
	int x, y, z;
	for (x = 0; x < CHUNK_SIZE; x++)
	{
		for (z = 0; z < CHUNK_SIZE; z++)
		{
			for (y = 0; y < CHUNK_HEIGHT; y++)
			{
				BlockFace face = CUBE_FACES[(int)FaceDirection::NegX];
				BlockType type = grid(x, z, y).type;

				if (type == BlockType::Air || type == BlockType::Undefined)
					continue;

				Vec3f topLeft = face.vertices[3] + Vec3f(x, y, z);
				Vec3f topRight;
				Vec3f bottomRight;
				Vec3f bottomLeft;

				int endY, endZ;
				for (endZ = z + 1; endZ < CHUNK_SIZE && grid(x, endZ, y).type == type; endZ++)
				{
					grid(x, endZ, y).type = BlockType::Undefined;
				}

				for (endY = y + 1; endY < CHUNK_HEIGHT; endY++)
				{
					bool passed = true;
					for (int cz = z; cz < endZ; cz++)
					{
						if (grid(x, cz, endY).type != type)
						{
							passed = false;
							break;
						}
					}
					if (passed)
					{
						for (int cz = z; cz < endZ; cz++)
							grid(x, cz, endY).type = BlockType::Undefined;
					}
					else
						break;
				}

				topRight = face.vertices[2] + Vec3f(x, endY - 1, z);
				bottomRight = face.vertices[1] + Vec3f(x, endY - 1, endZ - 1);
				bottomLeft = face.vertices[0] + Vec3f(x, y, endZ - 1);

				BlockFace mergedFace;
				mergedFace.vertices[3] = topLeft;
				mergedFace.vertices[2] = topRight;
				mergedFace.vertices[1] = bottomRight;
				mergedFace.vertices[0] = bottomLeft;
				mergedFace.normal = face.normal;
				for (int i = 0; i < 4; i++)
					mergedFace.uvs[i] = face.uvs[i];

				AddMergedFaceToMesh(mesh, mergedFace, type, FaceDirection::NegX, endZ - z, endY - y);

				y = endY - 1;
			}
		}
	}
}

void World::MergeNegY(Mat3D<Block>& grid, Mesh& mesh)
{
	int x, y, z;
	for (y = 0; y < CHUNK_HEIGHT; y++)
	{
		for (z = 0; z < CHUNK_SIZE; z++)
		{
			for (x = 0; x < CHUNK_SIZE; x++)
			{
				BlockFace face = CUBE_FACES[(int)FaceDirection::NegY];
				BlockType type = grid(y, z, x).type;

				if (type == BlockType::Air || type == BlockType::Undefined)
					continue;

				Vec3f topLeft = face.vertices[1] + Vec3f(x, y, z);
				Vec3f topRight;
				Vec3f bottomRight;
				Vec3f bottomLeft;

				int endZ, endX;
				for (endX = x + 1; endX < CHUNK_SIZE && grid(y, z, endX).type == type; endX++)
				{
					grid(y, z, endX).type = BlockType::Undefined;
				}

				for (endZ = z + 1; endZ < CHUNK_SIZE; endZ++)
				{
					bool passed = true;
					for (int cx = x; cx < endX; cx++)
					{
						if (grid(y, endZ, cx).type != type)
						{
							passed = false;
							break;
						}
					}
					if (passed)
					{
						for (int cx = x; cx < endX; cx++)
							grid(y, endZ, cx).type = BlockType::Undefined;
					}
					else
						break;
				}

				topRight = face.vertices[2] + Vec3f(endX - 1, y, z);
				bottomRight = face.vertices[3] + Vec3f(endX - 1, y, endZ - 1);
				bottomLeft = face.vertices[0] + Vec3f(x, y, endZ - 1);

				BlockFace mergedFace;
				mergedFace.vertices[0] = bottomLeft;
				mergedFace.vertices[3] = bottomRight;
				mergedFace.vertices[2] = topRight;
				mergedFace.vertices[1] = topLeft;
				mergedFace.normal = face.normal;
				for (int i = 0; i < 4; i++)
					mergedFace.uvs[i] = face.uvs[i];

				AddMergedFaceToMesh(mesh, mergedFace, type, FaceDirection::NegY, endX - x, endZ - z);

				x = endX - 1;
			}
		}
	}
}

void World::MergeNegZ(Mat3D<Block>& grid, Mesh& mesh)
{
	int x, y, z;
	for (z = 0; z < CHUNK_SIZE; z++)
	{
		for (x = 0; x < CHUNK_SIZE; x++)
		{
			for (y = 0; y < CHUNK_HEIGHT; y++)
			{
				BlockFace face = CUBE_FACES[(int)FaceDirection::NegZ];
				BlockType type = grid(z, x, y).type;

				if (type == BlockType::Air || type == BlockType::Undefined)
					continue;

				Vec3f topLeft = face.vertices[0] + Vec3f(x, y, z);
				Vec3f topRight;
				Vec3f bottomRight;
				Vec3f bottomLeft;

				int endX, endY;
				for (endY = y + 1; endY < CHUNK_HEIGHT && grid(z, x, endY).type == type; endY++)
				{
					grid(z, x, endY).type = BlockType::Undefined;
				}

				for (endX = x + 1; endX < CHUNK_SIZE; endX++)
				{
					bool passed = true;
					for (int cy = y; cy < endY; cy++)
					{
						if (grid(z, endX, cy).type != type)
						{
							passed = false;
							break;
						}
					}
					if (passed)
					{
						for (int cy = y; cy < endY; cy++)
							grid(z, endX, cy).type = BlockType::Undefined;
					}
					else
						break;
				}

				topRight = face.vertices[1] + Vec3f(x, endY - 1, z);
				bottomRight = face.vertices[2] + Vec3f(endX - 1, endY - 1, z);
				bottomLeft = face.vertices[3] + Vec3f(endX - 1, y, z);

				BlockFace mergedFace;
				mergedFace.vertices[0] = topLeft;
				mergedFace.vertices[1] = topRight;
				mergedFace.vertices[2] = bottomRight;
				mergedFace.vertices[3] = bottomLeft;
				mergedFace.normal = face.normal;
				for (int i = 0; i < 4; i++)
					mergedFace.uvs[i] = face.uvs[i];

				AddMergedFaceToMesh(mesh, mergedFace, type, FaceDirection::NegZ, endX - x, endY - y);

				y = endY - 1;
			}
		}
	}
}

void World::AddMergedFaceToMesh(
	Mesh& mesh,
	const BlockFace& face,
	BlockType type,
	FaceDirection dir,
	int uWraps,
	int vWraps
)
{
	int idx = mesh.vertices.size();

	for (int i = 0; i < 4; i++)
	{
		Vertex v;
		v.position = face.vertices[i];
		v.normal = face.normal;

		v.u = face.uvs[i].x * uWraps;
		v.v = face.uvs[i].y * vWraps;

		TileType tileType = TileType::Undefined;
		if (type == BlockType::Grass)
		{
			if (dir == FaceDirection::PosY) tileType = TileType::GrassTop;
			else if (dir == FaceDirection::NegY) tileType = TileType::Dirt;
			else tileType = TileType::GrassSide;
		}
		else if (type == BlockType::Dirt) tileType = TileType::Dirt;
		else if (type == BlockType::Stone) tileType = TileType::Stone;

		v.tileInfo = GetAtlasTileInfo(tileType);

		mesh.vertices.push_back(v);
	}

	mesh.indices.push_back(idx);
	mesh.indices.push_back(idx + 1);
	mesh.indices.push_back(idx + 2);

	mesh.indices.push_back(idx);
	mesh.indices.push_back(idx + 2);
	mesh.indices.push_back(idx + 3);
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

		Vec2f uvMin, uvMax;
		GetFaceUV(type, dir, uvMin, uvMax, 0, 0);
		v.u = face.uvs[i].x; // uvMin.x + face.uvs[i].x * (uvMax.x - uvMin.x);
		v.v = face.uvs[i].y; // uvMin.y + face.uvs[i].y * (uvMax.y - uvMin.y);

		TileType tileType = TileType::Undefined;
		if (type == BlockType::Grass)
		{
			if (dir == FaceDirection::PosY) tileType = TileType::GrassTop;
			else if (dir == FaceDirection::NegY) tileType = TileType::Dirt;
			else tileType = TileType::GrassSide;
		}
		else if (type == BlockType::Dirt) tileType = TileType::Dirt;
		else if (type == BlockType::Stone) tileType = TileType::Stone;

		v.tileInfo = GetAtlasTileInfo(tileType);

		mesh.vertices.push_back(v);
	}

	mesh.indices.push_back(startIndex + 0);
	mesh.indices.push_back(startIndex + 1);
	mesh.indices.push_back(startIndex + 2);

	mesh.indices.push_back(startIndex + 0);
	mesh.indices.push_back(startIndex + 2);
	mesh.indices.push_back(startIndex + 3);
}



bool World::FrustumTestChunk(const Vec3i& chunkPos, const Frustum& frustum) const 
{
	Vec3f min = Vec3f(
		chunkPos.x * CHUNK_SIZE,
		chunkPos.y * CHUNK_HEIGHT,
		chunkPos.z * CHUNK_SIZE
	);
	Vec3f max = Vec3f(
		(chunkPos.x + 1) * CHUNK_SIZE,
		(chunkPos.y + 1) * CHUNK_HEIGHT,
		(chunkPos.z + 1) * CHUNK_SIZE
	);

	for (int i = 0; i < 6; i++)
	{
		const Plane& plane = frustum.planes[i];
		Vec3f positiveVertex = min;
		if (plane.normal.x >= 0) positiveVertex.x = max.x;
		if (plane.normal.y >= 0) positiveVertex.y = max.y;
		if (plane.normal.z >= 0) positiveVertex.z = max.z;

		if (plane.normal.Dot(positiveVertex) + plane.d < 0)
		{
			return false;
		}
	}
	return true;
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

