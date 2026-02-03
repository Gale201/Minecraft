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

class World
{
public:
	void Update();

	void BuildRenderList(
		const Camera& camera,
		std::vector<RenderCommand>& outCommands
	);

	Block GetBlock(Vec3i pos) const;
	const Chunk* TryGetChunk(Vec3i pos) const;
	Chunk& GetChunk(Vec3i pos);
	Vec3i GetBlockChunkPosition(Vec3i pos) const;
	Vec3i GetChunkWorldPosition(Vec3i pos) const;
	Vec3i GetChunkPosition(Vec3i pos) const;
	Vec3i GetAbsoluteBlockPosition(Vec3i relPos, Chunk& chunk) const;

	const std::vector<const Chunk*> GetFrustumVisibleChunks(const Camera& camera) const;

	const std::unordered_map<Vec3i, Chunk, Vec3iHash>& GetChunks() const
	{
		return m_Chunks;
	}

	void SetBlock(Vec3i pos, BlockType type);
	void SetTextureAtlas(const Texture& texture)
	{
		m_TextureAtlas = texture;
	}

private:
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

	Frustum GetFrustumForCamera(const Camera& camera) const;
	bool FrustumTestChunk(const Vec3i& chunkPos, const Frustum& frustum) const;

	bool IsFaceVisible(Vec3i neighborPos);

	inline static int FloorDiv(int a, int b);
	inline static int FloorMod(int a, int b);

private:
	std::unordered_map<Vec3i, Chunk, Vec3iHash> m_Chunks;

	Texture m_TextureAtlas;
};
