#pragma once

#include <unordered_map>

#include "Chunk.h"
#include "Block.h"
#include "ChunkMesh.h"
#include "BlockFace.h"
#include "../math/Math.h"
#include "../render/Frustum.h"
#include "../render/Camera.h"
#include "../render/RenderCommand.h"
#include "ChunkLoader.h"
#include <queue>
#include "../thread/ThreadSystem.h"
#include "Raycast.h"

class World
{
public:
	World(unsigned int seed) : m_Seed(seed) {}
	~World() = default;

	void Update(ThreadSystem& threadSystem);

	void LoadAndUnloadChunks(Vec3f playerPosition);

	void BuildRenderList(
		const Camera& camera,
		std::vector<RenderCommand>& outCommands
	);

	bool Raycast(Vec3f origin, Vec3f direction, float maxDistance, WorldRaycastResult& result);

	Block GetBlock(Vec3i pos) const;
	Chunk* TryGetChunk(Vec3i pos) const;
	Chunk& GetChunk(Vec3i pos);
	Vec3i GetBlockChunkPosition(Vec3i pos) const;
	Vec3i GetChunkWorldPosition(Vec3i pos) const;
	Vec3i GetChunkPosition(Vec3i pos) const;
	inline Vec3i GetAbsoluteBlockPosition(Vec3i relPos, Chunk& chunk) const;

	const std::vector<const Chunk*> GetFrustumVisibleChunks(const Camera& camera) const;

	/* Getters & setters */

	const uint64_t GetSeed() const { return m_Seed; }

	const std::unordered_map<Vec3i, std::unique_ptr<Chunk>, Vec3iHash>& GetChunks() const
	{
		return m_Chunks;
	}

	void SetBlock(Vec3i pos, BlockType type);
	void SetTextureAtlas(const Texture& texture)
	{
		m_TextureAtlas = texture;
	}

private:
	/* Chunk loading functions */
	void LoadChunks(Vec3f playerPosition);
	void UnloadChunks(Vec3f playerPosition);

	/* Mesh building functions */
	void BuildChunkMesh(Chunk& chunk);

	void MergePosX(Mat3D<Block>& grid, Mesh& mesh);
	void MergePosY(Mat3D<Block>& grid, Mesh& mesh);
	void MergePosZ(Mat3D<Block>& grid, Mesh& mesh);
	void MergeNegX(Mat3D<Block>& grid, Mesh& mesh);
	void MergeNegY(Mat3D<Block>& grid, Mesh& mesh);
	void MergeNegZ(Mat3D<Block>& grid, Mesh& mesh);

	void AddMergedFaceToMesh(
		Mesh& mesh,
		const BlockFace& face,
		BlockType type,
		FaceDirection dir,
		int uWraps,
		int vWraps
	);

	void AddFaceToMesh(
		Mesh& mesh, 
		const BlockFace& face,
		Vec3i blockPos, 
		BlockType type,
		FaceDirection dir
	);

	void AddChunkToMeshingQueue(Chunk& chunk);

	bool FrustumTestChunk(const Vec3i& chunkPos, const Frustum& frustum) const;

	inline bool IsFaceVisible(
		Block block,
		Vec3i neighborPos,
		const Chunk& chunk,
		const Chunk* chunkPosX,
		const Chunk* chunkNegX,
		const Chunk* chunkPosZ,
		const Chunk* chunkNegZ
	) const;

	inline static int FloorDiv(int a, int b);
	inline static int FloorMod(int a, int b);

private:
	std::unordered_map<Vec3i, std::unique_ptr<Chunk>, Vec3iHash> m_Chunks;
	std::unordered_map<Vec3i, std::unordered_map<Vec3i, BlockType, Vec3i>, Vec3i> m_ModifiedChunks;
	std::queue<Vec3i> m_ChunkMeshingQueue;
	int m_MaxMeshedChunksPerFrame = 8;

	Texture m_TextureAtlas;

	const unsigned int m_Seed;
	
	Vec3f m_LastChunkLoadPosition;
	int m_ChunkLoadRadius = 8;
	ChunkLoader m_ChunkLoader{ m_Seed };
};
