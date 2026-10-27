#pragma once

#include <unordered_map>
#include <queue>
#include <deque>
#include <array>
#include <memory>

#include "Chunk.h"
#include "Block.h"
#include "BlockFace.h"
#include "ChunkLoader.h"
#include "Raycast.h"
#include "../math/Math.h"
#include "../render/Frustum.h"
#include "../render/Camera.h"
#include "../render/RenderCommand.h"
#include "../thread/ThreadSystem.h"
#include "../items/Item.h"
#include "../items/ItemRegistry.h"
#include "WorldStats.h"

namespace Minecraft
{
	class World
	{
	public:
		static const size_t MAX_ITEMS = 10000;

		struct BlockBreakingProgress
		{
			Block block;
			Vec3i blockPos;
			float progress = 0.0f;
		};

	public:
		World(
			const std::string& name,
			unsigned int seed,
			ItemRegistry& itemRegistry
		) : m_Name(name), m_Seed(seed), m_ItemRegistry(itemRegistry)
		{
			InitItems();
		}
		World(
			const std::string& name,
			unsigned int seed, 
			std::unordered_map<Vec3i, ModifiedChunk, Vec3iHash> modifiedChunks,
			ItemRegistry& itemRegistry
		) : m_Name(name), m_Seed(seed), m_ModifiedChunks(modifiedChunks), m_ItemRegistry(itemRegistry)
		{
			InitItems();
		}
		
		World(const World&) = delete;
		~World() = default;

		void Update(Core::ThreadSystem& threadSystem, float deltaTime);

		void LoadAndUnloadChunks(Vec3f playerPosition, Core::ThreadSystem& threadSystem);

		void BuildRenderList(
			const Camera& camera,
			std::vector<RenderCommand>& outCommands
		);

		bool Raycast(Vec3f origin, Vec3f direction, float maxDistance, WorldRaycastResult& result);

		void SpawnItem(Vec3f position, ItemStack stack);
		void AddItem(PhysicsBody body, ItemStack stack, float rotY, Vec3f spawnForce = Vec3f());
		void RemoveItem(ItemID id);

		Block GetBlock(Vec3i pos) const;
		Block* GetBlockPtr(Vec3i pos);
		bool SetBlock(Vec3i pos, BlockType type);
		Chunk* TryGetChunk(Vec3i pos) const;
		Chunk& GetChunk(Vec3i pos);
		Vec3i GetBlockPositionInChunk(Vec3i pos) const;
		Vec3i GetChunkWorldPosition(Vec3i pos) const;
		Vec3i GetChunkPosition(Vec3i pos) const;
		inline Vec3i GetAbsoluteBlockPosition(Vec3i relPos, Chunk& chunk) const;

		const std::vector<const Chunk*> GetFrustumVisibleChunks(const Camera& camera) const;

		const std::vector<Vec3i> GetSurroundingCollidableBlocks(Vec3i pos) const;

		/* Getters & setters */

		const std::string& GetName() const { return m_Name; }
		const unsigned int GetSeed() const { return m_Seed; }

		const std::unordered_map<Vec3i, std::unique_ptr<Chunk>, Vec3iHash>& GetChunks() const
		{
			return m_Chunks;
		}

		const std::unordered_map<Vec3i, ModifiedChunk, Vec3iHash>& GetModifiedChunks() const
		{
			return m_ModifiedChunks;
		}

		const std::vector<Item>& GetItems() const
		{
			return m_Items;
		}

		std::vector<Item>& GetItems()
		{
			return m_Items;
		}

		const std::vector<size_t>& GetActiveItemSlots() const
		{
			return m_ActiveItemSlots;
		}

		const std::vector<size_t>& GetFreeItemSlots() const
		{
			return m_FreeItemSlots;
		}

		const std::unordered_map<Vec3i, std::vector<size_t>, Vec3iHash>& GetItemsByChunk() const
		{
			return m_ItemsByChunk;
		}

		std::queue<std::pair<size_t, PhysicsBody>>& GetPhysicsAddQueue() { return m_PhysicsAddQueue; }

		size_t GetItemIndex(ItemID id) const
		{
			return m_ItemLookupTable.at(id);
		}

		ItemID GetCurrentItemID() const { return m_CurrentItemID; }

		int GetSeaLevel() const { return m_ChunkLoader.GetSeaLevel(); }

		float GetItemPickupCooldown() const { return m_ItemPickupCooldown; }

		const WorldStats& GetStats() const { return m_Stats; }

		void SetTextureAtlas(const Texture& texture)
		{
			m_TextureAtlas = &texture;
		}
		void SetBlockBreakingProgress(Block block, Vec3i pos, float progress)
		{
			m_BlockBreakingProgress = { block, pos, progress };
		}

	private:
		void InitItems();

		void AddToFluidFlow(Vec3i pos);
		void UpdateWaterBlockFlow();

		void SetWaterBlockFlowLevel(Vec3i pos, uint8_t level);
		uint8_t GetWaterBlockFlowLevel(Vec3i pos) const;

		/* Chunk loading functions */
		void LoadChunks(Vec3f playerPosition, Core::ThreadSystem& threadSystem);
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

		NeighbourFluidData GetNeighbourFluidData(Mat3D<Block>& grid, int x, int y, int z, BlockType type, FaceDirection faceDirection) const;

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
		const std::string& m_Name;
		const unsigned int m_Seed;

		std::unordered_map<Vec3i, std::unique_ptr<Chunk>, Vec3iHash> m_Chunks;
		std::unordered_map<Vec3i, ModifiedChunk, Vec3iHash> m_ModifiedChunks;
		std::queue<Vec3i> m_ChunkMeshingQueue;
		int m_MaxMeshedChunksPerFrame = 32;

		std::vector<Item> m_Items;
		std::vector<size_t> m_FreeItemSlots;
		std::vector<size_t> m_ActiveItemSlots;
		std::unordered_map<ItemID, size_t> m_ItemLookupTable;
		std::unordered_map<Vec3i, std::vector<size_t>, Vec3iHash> m_ItemsByChunk;
		ItemRegistry& m_ItemRegistry;

		std::queue<std::pair<size_t, PhysicsBody>> m_PhysicsAddQueue;

		const Texture* m_TextureAtlas = nullptr;

		Vec3f m_LastChunkLoadPosition;
		int m_ChunkLoadRadius = 16;
		ChunkLoader m_ChunkLoader{ m_Seed, m_ModifiedChunks };

		ItemID m_CurrentItemID = 0;
		float m_ItemBodyMass = 0.1f;
		float m_ItemPickupCooldown = 0.8f;

		BlockBreakingProgress m_BlockBreakingProgress;
		Mesh m_BlockBreakingMesh;

		std::queue<Vec3i> m_WaterBlockUpdateQueue;
		float m_WaterFlowUpdateInterval = 0.5f;
		float m_WaterFlowUpdateTimer = 0.0f;
		uint8_t m_MaxWaterFlowLevel = 8;

		WorldStats m_Stats;
	};
}
