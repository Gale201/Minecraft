#include "World.h"

#include "BlockFace.h"
#include "../core/Log.h"
#include "../render/Frustum.h"
#include "../render/Camera.h"
#include "../timer/Timer.h"
#include "../items/ItemRegistry.h"
#include <assert.h>

namespace Minecraft
{
	void World::Update(Core::ThreadSystem& threadSystem, float deltaTime)
	{
		// Update water flow
		m_WaterFlowUpdateTimer += deltaTime;
		while (m_WaterFlowUpdateTimer >= m_WaterFlowUpdateInterval)
		{
			UpdateWaterBlockFlow();

			m_WaterFlowUpdateTimer -= m_WaterFlowUpdateInterval;
		}

		/*
		 Chunk meshing 
		*/
		std::vector<Chunk*> chunksToBuild;
		chunksToBuild.reserve(m_MaxMeshedChunksPerFrame);

		// Fetching chunks that need meshing and submit their meshing to the thread system
		int meshedChunks = 0;
		while (!m_ChunkMeshingQueue.empty() && meshedChunks < m_MaxMeshedChunksPerFrame)
		{
			Vec3i chunkPos = m_ChunkMeshingQueue.front();
			auto it = m_Chunks.find(chunkPos);
			// Ignore if the chunk doesn't exist or doesn't need meshing
			if (it == m_Chunks.end() || it->second->state.load(std::memory_order_acquire) != ChunkState::NeedsMeshing)
			{
				m_ChunkMeshingQueue.pop();
				continue;
			}
		
			// Submit the BuildChunkMesh job to the thread system
			Chunk* chunk = it->second.get();
			chunksToBuild.push_back(chunk);
			m_ChunkMeshingQueue.pop();
			meshedChunks++;
		}

		threadSystem.ParallelFor(
			(int)chunksToBuild.size(),
			[this, chunksToBuild](int index)
			{
				BuildChunkMesh(*chunksToBuild[index]);
			}
		);

		// Check if there are any chunks that are meshed, and move their buildMesh buffer to the main mesh
		// This needs to be on the main thread to avoid vector thread collisions
		for (auto& [pos, chunk] : m_Chunks)
		{
			if (chunk->state.load(std::memory_order_acquire) == ChunkState::Ready)
			{
				chunk->mesh = std::move(chunk->buildMesh);
				chunk->state.store(ChunkState::Generated, std::memory_order_release);
			}
		}

		for (Item& item : m_Items)
			item.pickupCooldown = std::fmaxf(0.0f, item.pickupCooldown - deltaTime);

		m_BlockBreakingProgress.progress = 0.0f;
	}

	void World::LoadAndUnloadChunks(Vec3f playerPosition, Core::ThreadSystem& threadSystem)
	{
		LoadChunks(playerPosition, threadSystem);
		UnloadChunks(playerPosition);
	}

	void World::BuildRenderList(const Camera& camera, std::vector<RenderCommand>& outCommands)
	{
		const std::vector<const Chunk*> visibleChunks = GetFrustumVisibleChunks(camera);
		for (const Chunk* chunk : visibleChunks)
		{
			Vec3i chunkPos = chunk->position;

			// Blocks
			Transform transform;
			transform.SetPosition(GetChunkWorldPosition(chunkPos).toVec3f());

			for (int dir = 0; dir < 6; dir++)
			{
				if (chunk->mesh.meshes[dir].indices.size() > 0)
				{
					outCommands.emplace_back(
						&chunk->mesh.meshes[dir],
						transform,
						*m_TextureAtlas
						);
				}
			}

			// Block breaking animation
			Vec3i breakingBlockPos = m_BlockBreakingProgress.blockPos;
			if (
				GetChunkPosition(breakingBlockPos) == chunkPos && 
				m_BlockBreakingProgress.progress > 0.0f &&
				m_BlockBreakingProgress.block.IsBreakable() &&
				m_BlockBreakingProgress.block.IsFullBlock()
			)
			{
				int tileX = static_cast<int>(std::floorf(m_BlockBreakingProgress.progress * 9.f));
				transform.SetPosition(breakingBlockPos.toVec3f());
				m_BlockBreakingMesh = CreateBlockMesh(
					CreateAtlasTileInfo(
						tileX, 15, true, false
					),
					1.001f,
					Vec3f(-0.0005f, -0.0005f, -0.0005f)
				);
				outCommands.emplace_back(
					&m_BlockBreakingMesh,
					transform,
					*m_TextureAtlas
				);
			}

			// Items
			auto it = m_ItemsByChunk.find(chunkPos);
			if (it != m_ItemsByChunk.end())
			{
				const auto& indices = it->second;

				for (size_t index : indices)
				{
					const Item& item = m_Items[index];
					if (!item.body)
						continue;

					ItemDefinition definition = m_ItemRegistry.GetItemDefinition(item.stack.type);

					transform.SetPosition(item.body->position + item.body->size * 0.5f);
					transform.SetRotation(item.rotation);

					outCommands.emplace_back(
						definition.mesh,
						transform,
						*definition.texture
					);
				}
			}
		}
	}

	bool World::Raycast(Vec3f origin, Vec3f direction, float maxDistance, WorldRaycastResult& result)
	{
		direction = direction.normalized();

		Vec3i originBlock = Vec3i::floor(origin);
		int stepX = (direction.x > 0.0f) ? 1 : -1;
		int stepY = (direction.y > 0.0f) ? 1 : -1;
		int stepZ = (direction.z > 0.0f) ? 1 : -1;

		float tDeltaX = std::abs(1.0f / direction.x);
		float tDeltaY = std::abs(1.0f / direction.y);
		float tDeltaZ = std::abs(1.0f / direction.z);

		float tMaxX = (stepX > 0) ? (std::floor(origin.x) + 1 - origin.x) : (origin.x - std::floor(origin.x));
		float tMaxY = (stepY > 0) ? (std::floor(origin.y) + 1 - origin.y) : (origin.y - std::floor(origin.y));
		float tMaxZ = (stepZ > 0) ? (std::floor(origin.z) + 1 - origin.z) : (origin.z - std::floor(origin.z));
		tMaxX *= tDeltaX;
		tMaxY *= tDeltaY;
		tMaxZ *= tDeltaZ;

		float distance = 0.0f;
		Vec3i pos = { originBlock.x, originBlock.y, originBlock.z };
		Vec3f normal;
		while (distance <= maxDistance)
		{
			Block block = GetBlock(pos);
			BlockBoundingBox box = GetBlockBoundingBox(block.type);
			if (block.isCollidableWithRaycasts())
			{
				if (!block.IsFullBlock())
				{
					Vec3f min = pos.toVec3f() + box.offset;
					Vec3f max = min + box.size;

					float t;
					if (RaycastAgainstBox(origin, direction, min, max, t) && t <= maxDistance)
					{
						result = { pos, normal, block.type, t };
						return true;
					}
				}
				else
				{
					result = { pos, normal, block.type, distance };
					return true;
				}
			}

			if (tMaxX < tMaxY)
			{
				if (tMaxX < tMaxZ)
				{
					pos.x += stepX;
					tMaxX += tDeltaX;
					normal = Vec3f(static_cast<float>(-stepX), 0.0f, 0.0f);
				}
				else
				{
					pos.z += stepZ;
					tMaxZ += tDeltaZ;
					normal = Vec3f(0.0f, 0.0f, static_cast<float>(-stepZ));
				}
			}
			else
			{
				if (tMaxY < tMaxZ)
				{
					pos.y += stepY;
					tMaxY += tDeltaY;
					normal = Vec3f(0.0f, static_cast<float>(-stepY), 0.0f);
				}
				else
				{
					pos.z += stepZ;
					tMaxZ += tDeltaZ;
					normal = Vec3f(0.0f, 0.0f, static_cast<float>(-stepZ));
				}
			}

			distance = (pos - originBlock).length();
		}

		return false;
	}

	void World::SpawnItem(Vec3f position, ItemStack stack)
	{
		Vec3i chunkPosition = GetChunkPosition(Vec3i::floor(position));

		PhysicsBody body = PhysicsBody(position, m_ItemRegistry.GetItemHitboxSize());
		Vec3f spawnForce = Vec3f(
			(static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5f),
			1.0f,
			(static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5f)
		);

		float rotY = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * PI;

		AddItem(body, stack, rotY, spawnForce * 20.0f);
	}

	void World::AddItem(PhysicsBody body, ItemStack stack, float rotY, Vec3f spawnForce)
	{
		Vec3i chunkPosition = GetChunkPosition(Vec3i::floor(body.position));

		if (m_FreeItemSlots.empty())
		{
			LOG_WARNING("Global item storage is full!");
			return;
		}

		size_t index = m_FreeItemSlots.back();
		m_FreeItemSlots.pop_back();

		if (index >= m_Items.size())
			m_Items.resize(index + 1);

		m_Items[index] = Item(m_CurrentItemID, stack, nullptr, Vec3f(0.0f, rotY, 0.0f));
		m_Items[index].chunkPos = chunkPosition;
		m_ItemLookupTable.emplace(m_CurrentItemID, index);
		m_ActiveItemSlots.push_back(index);
		m_ItemsByChunk[chunkPosition].push_back(index);

		m_CurrentItemID++;

		body.mass = m_ItemBodyMass;
		body.size = m_ItemRegistry.GetItemHitboxSize();
		body.AddForce(spawnForce);
		m_PhysicsAddQueue.push(std::make_pair(index, body));
	}

	void World::RemoveItem(ItemID id)
	{
		auto it = m_ItemLookupTable.find(id);
		if (it == m_ItemLookupTable.end())
			return;

		size_t index = it->second;
		Item& item = m_Items[index];

		// Remove item from chunk list
		auto& chunkItems = m_ItemsByChunk[item.chunkPos];
		for (size_t i = 0; i < chunkItems.size(); i++)
		{
			if (chunkItems[i] == index)
			{
				std::swap(chunkItems[i], chunkItems.back());
				chunkItems.pop_back();
				break;
			}
		}

		// Remove from active items
		for (size_t i = 0; i < m_ActiveItemSlots.size(); i++)
		{
			if (m_ActiveItemSlots[i] == index)
			{
				std::swap(m_ActiveItemSlots[i], m_ActiveItemSlots.back());
				m_ActiveItemSlots.pop_back();
				break;
			}
		}

		m_FreeItemSlots.push_back(index);

		m_ItemLookupTable.erase(id);

		item.stack.size = 0;
		item.body = nullptr;
	}

	Vec3i World::GetChunkPosition(Vec3i pos) const
	{
		return Vec3i(
			FloorDiv(pos.x, CHUNK_SIZE),
			FloorDiv(pos.y, CHUNK_HEIGHT),
			FloorDiv(pos.z, CHUNK_SIZE)
		);
	}

	Vec3i World::GetBlockPositionInChunk(Vec3i pos) const
	{
		return Vec3i(
			FloorMod(pos.x, CHUNK_SIZE),
			FloorMod(pos.y, CHUNK_HEIGHT),
			FloorMod(pos.z, CHUNK_SIZE)
		);
	}

	inline Vec3i World::GetAbsoluteBlockPosition(Vec3i relPos, Chunk& chunk) const
	{
		return {
			relPos.x + chunk.position.x * CHUNK_SIZE,
			relPos.y + chunk.position.y * CHUNK_HEIGHT,
			relPos.z + chunk.position.z * CHUNK_SIZE
		};
	}

	Vec3i World::GetChunkWorldPosition(Vec3i pos) const
	{
		return Vec3i(
			pos.x * CHUNK_SIZE,
			pos.y * CHUNK_HEIGHT,
			pos.z * CHUNK_SIZE
		);
	}

	Chunk* World::TryGetChunk(Vec3i pos) const
	{
		auto it = m_Chunks.find(pos);
		if (it == m_Chunks.end())
			return nullptr;

		return it->second.get();
	}

	Chunk& World::GetChunk(Vec3i pos)
	{
		auto it = m_Chunks.find(pos);
		if (it != m_Chunks.end()) return *it->second;

		std::unique_ptr<Chunk> chunk = std::make_unique<Chunk>();
		chunk->position = pos;

		m_ChunkLoader.LoadChunk(*chunk);
	
		m_Chunks.emplace(pos, std::move(chunk));
		AddChunkToMeshingQueue(*m_Chunks[pos]);
		return *m_Chunks[pos];
	}

	Block World::GetBlock(Vec3i pos) const
	{
		Vec3i chunkPos = GetChunkPosition(pos);
		const Chunk* chunk = TryGetChunk(chunkPos);
		if (chunk == nullptr) 
			return Block(BlockType::Undefined);
	
		Vec3i blockPos = GetBlockPositionInChunk(pos);
		return chunk->blocks[blockPos.x][blockPos.y][blockPos.z];
	}

	Block* World::GetBlockPtr(Vec3i pos)
	{
		Vec3i chunkPos = GetChunkPosition(pos);
		Chunk* chunk = TryGetChunk(chunkPos);
		if (chunk == nullptr)
			return nullptr;

		Vec3i blockPos = GetBlockPositionInChunk(pos);
		return &chunk->blocks[blockPos.x][blockPos.y][blockPos.z];
	}

	bool World::SetBlock(Vec3i pos, BlockType type)
	{
		if (pos.y >= CHUNK_HEIGHT) return false;

		// Get the chunk that the block is in
		Vec3i chunkPos = GetChunkPosition(pos);
		Chunk& chunk = GetChunk(chunkPos);

		// Get relative block position in the chunk
		Block block(type);
		Vec3i blockPos = GetBlockPositionInChunk(pos);
		chunk.blocks[blockPos.x][blockPos.y][blockPos.z] = block;

		// Add the chunk and the block to the modified chunks
		m_ModifiedChunks[chunkPos].blocks[blockPos] = block;

		// Add chunk and any neighbor chunks that would need remeshing to the meshing queue

		AddChunkToMeshingQueue(chunk);

		if (blockPos.x == 0)
		{
			Chunk& neighbor = GetChunk({ chunkPos.x - 1, chunkPos.y, chunkPos.z });
			AddChunkToMeshingQueue(neighbor);
		}
		else if (blockPos.x == CHUNK_SIZE - 1)
		{
			Chunk& neighbor = GetChunk({ chunkPos.x + 1, chunkPos.y, chunkPos.z });
			AddChunkToMeshingQueue(neighbor);
		}
		if (blockPos.z == 0)
		{
			Chunk& neighbor = GetChunk({ chunkPos.x, chunkPos.y, chunkPos.z - 1 });
			AddChunkToMeshingQueue(neighbor);
		}
		else if (blockPos.z == CHUNK_SIZE - 1)
		{
			Chunk& neighbor = GetChunk({ chunkPos.x, chunkPos.y, chunkPos.z + 1 });
			AddChunkToMeshingQueue(neighbor);
		}

		AddToFluidFlow(pos);

		return true;
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
				visible.push_back(chunk.get());
			}
		}
		return visible;
	}

	const std::vector<Vec3i> World::GetSurroundingCollidableBlocks(Vec3i pos) const
	{
		std::vector<Vec3i> blocks;
		blocks.reserve(64);

		for (int x = pos.x - 1; x <= pos.x + 1; x++)
		{
			for (int y = pos.y - 2; y <= pos.y + 2; y++)
			{
				for (int z = pos.z - 1; z <= pos.z + 1; z++)
				{
					Vec3i blockPos = Vec3i(x, y, z);
					if (GetBlock(blockPos).IsCollidable())
						blocks.emplace_back(x, y, z);
				}
			}
		}

		return blocks;
	}

	/* Private functions */

	void World::InitItems()
	{
		m_Items.resize(MAX_ITEMS);

		for (size_t i = 0; i < MAX_ITEMS; i++)
			m_FreeItemSlots.push_back(i);
	}

	void World::AddToFluidFlow(Vec3i pos)
	{
		Block block = GetBlock(pos);
		Block blockPosX = GetBlock(pos + Vec3i(1, 0, 0));
		Block blockNegX = GetBlock(pos + Vec3i(-1, 0, 0));
		Block blockPosY = GetBlock(pos + Vec3i(0, 1, 0));
		Block blockNegY = GetBlock(pos + Vec3i(0, -1, 0));
		Block blockPosZ = GetBlock(pos + Vec3i(0, 0, 1));
		Block blockNegZ = GetBlock(pos + Vec3i(0, 0, -1));

		if (
			block.type == BlockType::Water ||
			blockPosX.type == BlockType::Water ||
			blockNegX.type == BlockType::Water ||
			blockPosZ.type == BlockType::Water ||
			blockNegZ.type == BlockType::Water ||
			blockPosY.type == BlockType::Water
			)
		{
			m_WaterBlockUpdateQueue.push(pos);
		}
		else if (block.type != BlockType::Water)
		{
			if (blockPosX.type == BlockType::Water)
				m_WaterBlockUpdateQueue.push(pos + Vec3i(1, 0, 0));
			if (blockNegX.type == BlockType::Water)
				m_WaterBlockUpdateQueue.push(pos + Vec3i(-1, 0, 0));
			if (blockPosZ.type == BlockType::Water)
				m_WaterBlockUpdateQueue.push(pos + Vec3i(0, 0, 1));
			if (blockNegZ.type == BlockType::Water)
				m_WaterBlockUpdateQueue.push(pos + Vec3i(0, 0, -1));
			if (blockNegY.type == BlockType::Water)
				m_WaterBlockUpdateQueue.push(pos + Vec3i(0, -1, 0));

		}
	}

	void World::UpdateWaterBlockFlow()
	{
		struct WaterUpdate
		{
			Vec3i pos;
			uint8_t flowLevel;
		};

		std::vector<WaterUpdate> blocksToUpdate;
		while (!m_WaterBlockUpdateQueue.empty())
		{
			Vec3i pos = m_WaterBlockUpdateQueue.front();
			m_WaterBlockUpdateQueue.pop();

			uint8_t blockFlowLevel = GetWaterBlockFlowLevel(pos);
			if (blockFlowLevel >= m_MaxWaterFlowLevel)
				continue;

			Block block = GetBlock(pos);
			if (block.type != BlockType::Water)
				continue;

			Vec3i belowPos = pos + Vec3i(0, -1, 0);
			Block blockBelow = GetBlock(belowPos);
			if (blockBelow.type == BlockType::Air || (blockBelow.type == BlockType::Water && blockBelow.fluidLevel > 1))
			{
				blocksToUpdate.push_back({ belowPos, 1 });
				if (blockFlowLevel > 0)
					continue;
			}
			
			std::array<Vec3i, 4> directions = { Vec3i(1, 0, 0), Vec3i(-1, 0, 0), Vec3i(0, 0, 1), Vec3i(0, 0, -1) };
			int minDistanceToWhole = m_MaxWaterFlowLevel;
			int dirValues[4];
			for (int i = 0; i < 4; i++)
			{
				dirValues[i] = m_MaxWaterFlowLevel;
				for (int j = 1; j < 5; j++)
				{
					if (GetBlock(pos + directions[i] * j).type != BlockType::Air)
						break;
					Block tempBlock = GetBlock(pos + directions[i] * j + Vec3i(0, -1, 0));
					if (tempBlock.type == BlockType::Air || tempBlock.IsFluid() || tempBlock.IsWaterBreakable())
					{
						minDistanceToWhole = std::min(minDistanceToWhole, j);
						dirValues[i] = j;
						break;
					}
				}
			}

			uint8_t validDirections = 0;
			for (int i = 0; i < 4; i++)
			{
				if (dirValues[i] == minDistanceToWhole)
					validDirections |= (1 << i);
			}

			for (int i = 0; i < 4; i++)
			{
				if (((validDirections >> i) & 1) == 0)
					continue;

				Vec3i& dir = directions[i];
				Vec3i neighborPos = pos + dir;
				Block neighborBlock = GetBlock(neighborPos);

				uint8_t newFlowLevel = blockFlowLevel + 1;
				if (neighborBlock.type == BlockType::Air || (neighborBlock.type == BlockType::Water && neighborBlock.fluidLevel > newFlowLevel))
				{
					if (newFlowLevel < m_MaxWaterFlowLevel)
						blocksToUpdate.push_back({ neighborPos, newFlowLevel });
				}
			}
		}

		for (const WaterUpdate& update : blocksToUpdate)
		{
			Block currentBlock = GetBlock(update.pos);
			if (currentBlock.type == BlockType::Air || (currentBlock.type == BlockType::Water && currentBlock.fluidLevel > update.flowLevel))
			{
				SetBlock(update.pos, BlockType::Water);
				SetWaterBlockFlowLevel(update.pos, update.flowLevel);

				m_WaterBlockUpdateQueue.push(update.pos);

				Vec3i posXPos = update.pos + Vec3i(1, 0, 0);
				Vec3i negXPos = update.pos + Vec3i(-1, 0, 0);
				Vec3i posZPos = update.pos + Vec3i(0, 0, 1);
				Vec3i negZPos = update.pos + Vec3i(0, 0, -1);
				Vec3i posXposZPos = update.pos + Vec3i(1, 0, 1);
				Vec3i negXposZPos = update.pos + Vec3i(-1, 0, 1);
				Vec3i posXnegZPos = update.pos + Vec3i(1, 0, -1);
				Vec3i negXnegZPos = update.pos + Vec3i(-1, 0, -1);

				Block* blockPosX = GetBlockPtr(posXPos);
				Block* blockNegX = GetBlockPtr(negXPos);
				Block* blockPosZ = GetBlockPtr(posZPos);
				Block* blockNegZ = GetBlockPtr(negZPos);
				Block* blockPosXposZ = GetBlockPtr(posXposZPos);
				Block* blockNegXposZ = GetBlockPtr(negXposZPos);
				Block* blockPosXnegZ = GetBlockPtr(posXnegZPos);
				Block* blockNegXnegZ = GetBlockPtr(negXnegZPos);

				if (blockPosX)
				{
					blockPosX->negXnegZFluidLevel = std::min(blockPosX->negXnegZFluidLevel, update.flowLevel);
					blockPosX->negXposZFluidLevel = std::min(blockPosX->negXposZFluidLevel, update.flowLevel);
					m_ModifiedChunks[GetChunkPosition(posXPos)].blocks[GetBlockPositionInChunk(posXPos)] = *blockPosX;
				}
				if (blockNegX)
				{
					blockNegX->posXnegZFluidLevel = std::min(blockNegX->posXnegZFluidLevel, update.flowLevel);
					blockNegX->posXposZFluidLevel = std::min(blockNegX->posXposZFluidLevel, update.flowLevel);
					m_ModifiedChunks[GetChunkPosition(negXPos)].blocks[GetBlockPositionInChunk(negXPos)] = *blockNegX;
				}
				if (blockPosZ)
				{
					blockPosZ->negXnegZFluidLevel = std::min(blockPosZ->negXnegZFluidLevel, update.flowLevel);
					blockPosZ->posXnegZFluidLevel = std::min(blockPosZ->posXnegZFluidLevel, update.flowLevel);
					m_ModifiedChunks[GetChunkPosition(posZPos)].blocks[GetBlockPositionInChunk(posZPos)] = *blockPosZ;
				}
				if (blockNegZ)
				{
					blockNegZ->negXposZFluidLevel = std::min(blockNegZ->negXposZFluidLevel, update.flowLevel);
					blockNegZ->posXposZFluidLevel = std::min(blockNegZ->posXposZFluidLevel, update.flowLevel);
					m_ModifiedChunks[GetChunkPosition(negZPos)].blocks[GetBlockPositionInChunk(negZPos)] = *blockNegZ;
				}
				if (blockPosXposZ)
				{
					blockPosXposZ->negXnegZFluidLevel = std::min(blockPosXposZ->negXnegZFluidLevel, update.flowLevel);
					m_ModifiedChunks[GetChunkPosition(posXposZPos)].blocks[GetBlockPositionInChunk(posXposZPos)] = *blockPosXposZ;
				}
				if (blockNegXposZ)
				{
					blockNegXposZ->posXnegZFluidLevel = std::min(blockNegXposZ->posXnegZFluidLevel, update.flowLevel);
					m_ModifiedChunks[GetChunkPosition(negXposZPos)].blocks[GetBlockPositionInChunk(negXposZPos)] = *blockNegXposZ;
				}
				if (blockPosXnegZ)
				{
					blockPosXnegZ->negXposZFluidLevel = std::min(blockPosXnegZ->negXposZFluidLevel, update.flowLevel);
					m_ModifiedChunks[GetChunkPosition(posXnegZPos)].blocks[GetBlockPositionInChunk(posXnegZPos)] = *blockPosXnegZ;
				}
				if (blockNegXnegZ)
				{
					blockNegXnegZ->posXposZFluidLevel = std::min(blockNegXnegZ->posXposZFluidLevel, update.flowLevel);
					m_ModifiedChunks[GetChunkPosition(negXnegZPos)].blocks[GetBlockPositionInChunk(negXnegZPos)] = *blockNegXnegZ;
				}
			}
		}
	}

	void World::SetWaterBlockFlowLevel(Vec3i pos, uint8_t level)
	{
		Block* block = GetBlockPtr(pos);
		if (!block) return;
		block->fluidLevel = level;
		m_ModifiedChunks[GetChunkPosition(pos)].blocks[GetBlockPositionInChunk(pos)] = *block;
	}

	uint8_t World::GetWaterBlockFlowLevel(Vec3i pos) const
	{
		Block block = GetBlock(pos);
		if (block.type != BlockType::Water)
			return 0;
		return block.fluidLevel;
	}

	/* Chunk loading */

	void World::LoadChunks(Vec3f playerPosition, Core::ThreadSystem& threadSystem)
	{
		if (playerPosition == m_LastChunkLoadPosition) return;

		int playerX = FloorDiv(static_cast<int>(std::floor(playerPosition.x)), CHUNK_SIZE);
		int playerZ = FloorDiv(static_cast<int>(std::floor(playerPosition.z)), CHUNK_SIZE);

		std::vector<Vec3i> chunksToLoad;
		for (int x = playerX - m_ChunkLoadRadius; x <= playerX + m_ChunkLoadRadius; x++) 
		{ 
			for (int z = playerZ - m_ChunkLoadRadius; z <= playerZ + m_ChunkLoadRadius; z++)
			{
				//if ((x - playerX) * (x - playerX) + (z - playerZ) * (z - playerZ) > m_ChunkLoadRadius * m_ChunkLoadRadius) continue;

				Vec3i chunkPos = Vec3i(x, 0, z);
				if (m_Chunks.find(chunkPos) != m_Chunks.end())  // If the chunk is already loaded, skip it
					continue;

				chunksToLoad.push_back(chunkPos);
			}
		}

		std::vector<std::unique_ptr<Chunk>> loadedChunks(chunksToLoad.size());
		threadSystem.ParallelFor(
			(int)chunksToLoad.size(),
			[&](int index)
			{
				Vec3i chunkPos = chunksToLoad[index];

				// Load the chunk and add it to the world
				std::unique_ptr<Chunk> chunk = std::make_unique<Chunk>();
				chunk->position = chunkPos;

				m_ChunkLoader.LoadChunk(*chunk);

				loadedChunks[index] = std::move(chunk);
			}
		);

		for (int i = 0; i < chunksToLoad.size(); i++)
		{
			Vec3i chunkPos = chunksToLoad[i];

			m_Chunks.emplace(chunkPos, std::move(loadedChunks[i]));
			AddChunkToMeshingQueue(*m_Chunks[chunkPos]);

			auto it = m_ItemsByChunk.find(chunkPos);
			if (it != m_ItemsByChunk.end())
			{
				const auto& indices = it->second;

				for (size_t index : indices)
				{
					m_Items[index].body->pausePhysics = false;
				}
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

			/*if ((chunkPos.x - playerX) * (chunkPos.x - playerX) + (chunkPos.z - playerZ) * (chunkPos.z - playerZ) > m_ChunkLoadRadius * m_ChunkLoadRadius)
				chunksToUnload.push_back(chunkPos);*/
		}
		for (const Vec3i& chunkPos : chunksToUnload)
		{ 
			m_Chunks.erase(chunkPos);

			auto it = m_ItemsByChunk.find(chunkPos);
			if (it != m_ItemsByChunk.end())
			{
				const auto& indices = it->second;

				for (size_t index : indices)
				{
					m_Items[index].body->pausePhysics = false;
				}
			}
		}

		m_LastChunkLoadPosition = playerPosition;
	}

	/* Chunk mesh building */

	void World::BuildChunkMesh(Chunk& chunk)
	{
		if (chunk.state.load(std::memory_order_acquire) == ChunkState::Meshing) return;

		chunk.state.store(ChunkState::Meshing, std::memory_order_release);

		// Clear the mesh
		for (Mesh& meshPart : chunk.buildMesh.meshes)
		{
			meshPart.vertices.clear();
			meshPart.indices.clear();
		}

		// Get the neighbor chunks
		Chunk* chunkPosX = TryGetChunk({ chunk.position.x + 1, chunk.position.y, chunk.position.z });
		Chunk* chunkNegX = TryGetChunk({ chunk.position.x - 1, chunk.position.y, chunk.position.z });
		Chunk* chunkPosZ = TryGetChunk({ chunk.position.x, chunk.position.y, chunk.position.z + 1 });
		Chunk* chunkNegZ = TryGetChunk({ chunk.position.x, chunk.position.y, chunk.position.z - 1 });

		// Temporary chunks, used if the neighbor chunks doesn't exist
		Chunk tempPosX{ { chunk.position.x + 1, chunk.position.y, chunk.position.z } };
		Chunk tempNegX{ { chunk.position.x - 1, chunk.position.y, chunk.position.z } };
		Chunk tempPosZ{ { chunk.position.x, chunk.position.y, chunk.position.z + 1 } };
		Chunk tempNegZ{ { chunk.position.x, chunk.position.y, chunk.position.z - 1 } };

		// If the neighbor chunks don't exist load them
		if (!chunkPosX)
		{
			m_ChunkLoader.LoadChunk(tempPosX);
			chunkPosX = &tempPosX;
		}
		else
			chunkPosX = chunkPosX;
	
		if (!chunkNegX)
		{
			m_ChunkLoader.LoadChunk(tempNegX);
			chunkNegX = &tempNegX;
		}
		else
			chunkNegX = chunkNegX;
	
		if (!chunkPosZ)
		{
			m_ChunkLoader.LoadChunk(tempPosZ);
			chunkPosZ = &tempPosZ;
		}
		else
			chunkPosZ = chunkPosZ;
	
		if (!chunkNegZ)
		{
			m_ChunkLoader.LoadChunk(tempNegZ);
			chunkNegZ = &tempNegZ;
		}
		else
			chunkNegZ = chunkNegZ;

		// 2d mesh slices of the chunk
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

					if (!block.IsFullBlock())
					{
						int i = 0;
						for (FaceDirection dir : X_FACE_DIRECTIONS)
						{
							BlockFace face = GetBlockFace(block.type, dir, false);
							AddFaceToMesh(chunk.buildMesh.meshes[i++], face, Vec3i(x, y, z), block.type, dir);
						}
						continue;
					}

					for (FaceDirection dir : FACE_DIRECTIONS)
					{
						Vec3i neighborPos = Vec3i(x, y, z) + GetCubeNeighborOffset(dir);
						if (IsFaceVisible(block, neighborPos, chunk, chunkPosX, chunkNegX, chunkPosZ, chunkNegZ))
						{
							if (dir == FaceDirection::PosY)
								gridPosY(y, x, z) = block;
							else if (dir == FaceDirection::PosX)
								gridPosX(x, y, z) = block;
							else if (dir == FaceDirection::PosZ)
								gridPosZ(z, y, x) = block;
							else if (dir == FaceDirection::NegX)
								gridNegX(x, z, y) = block;
							else if (dir == FaceDirection::NegY)
								gridNegY(y, z, x) = block;
							else if (dir == FaceDirection::NegZ)
								gridNegZ(z, x, y) = block;
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

		MergePosX(gridPosX, chunk.buildMesh.meshes[0]);
		MergeNegX(gridNegX, chunk.buildMesh.meshes[1]);
		MergePosY(gridPosY, chunk.buildMesh.meshes[2]);
		MergeNegY(gridNegY, chunk.buildMesh.meshes[3]);
		MergePosZ(gridPosZ, chunk.buildMesh.meshes[4]);
		MergeNegZ(gridNegZ, chunk.buildMesh.meshes[5]);

		chunk.state.store(ChunkState::Ready, std::memory_order_release);		
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
					BlockType type = grid(x, y, z).type;
					bool isTop = false;
					uint8_t fluidLevel = grid(x, y, z).fluidLevel;
					if (y < CHUNK_HEIGHT - 1 && grid(x, y + 1, z).type != type) isTop = true;
					
					NeighbourFluidData neighbourFluidData = GetNeighbourFluidData(grid, x, y, z, type, FaceDirection::PosX);
					
					BlockFace face = GetBlockFace(type, FaceDirection::PosX, isTop, fluidLevel, neighbourFluidData);
					
					if (type == BlockType::Air || type == BlockType::Undefined)
						continue;

					Vec3f topLeft = face.vertices[0] + Vec3f(x, y, z);
					Vec3f topRight;
					Vec3f bottomRight;
					Vec3f bottomLeft;

					int endZ, endY;
					for (endY = y + 1; endY < CHUNK_HEIGHT && grid(x, endY, z).type == type && grid(x, endY, z).fluidLevel == fluidLevel; endY++)
					{
						grid(x, endY, z).type = BlockType::Undefined;
					}

					for (endZ = z + 1; endZ < CHUNK_SIZE; endZ++)
					{
						bool passed = true;
						for (int cy = y; cy < endY; cy++)
						{
							if (grid(x, cy, endZ).type != type || grid(x, cy, endZ).fluidLevel != fluidLevel)
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
					BlockType type = grid(y, x, z).type;
					bool isTop = false;
					uint8_t fluidLevel = grid(y, x, z).fluidLevel;
					if (y < CHUNK_HEIGHT - 1 && grid(y + 1, x, z).type != type) isTop = true;
					
					NeighbourFluidData neighbourFluidData = GetNeighbourFluidData(grid, x, y, z, type, FaceDirection::PosY);
					
					BlockFace face = GetBlockFace(type, FaceDirection::PosY, isTop, fluidLevel, neighbourFluidData);

					if (type == BlockType::Air || type == BlockType::Undefined)
						continue;

					Vec3f topLeft = face.vertices[0] + Vec3f(x, y, z);
					Vec3f topRight;
					Vec3f bottomRight;
					Vec3f bottomLeft;

					int endX, endZ;
					for (endZ = z + 1; endZ < CHUNK_SIZE && grid(y, x, endZ).type == type && grid(y, x, endZ).fluidLevel == fluidLevel; endZ++)
					{
						grid(y, x, endZ).type = BlockType::Undefined;
					}

					for (endX = x + 1; endX < CHUNK_SIZE; endX++)
					{
						bool passed = true;
						for (int cz = z; cz < endZ; cz++)
						{
							if (grid(y, endX, cz).type != type || grid(y, endX, cz).fluidLevel != fluidLevel)
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
					BlockType type = grid(z, y, x).type;
					bool isTop = false;
					uint8_t fluidLevel = grid(z, y, x).fluidLevel;
					if (y < CHUNK_HEIGHT - 1 && grid(z, y + 1, x).type != type) isTop = true;
					
					NeighbourFluidData neighbourFluidData = GetNeighbourFluidData(grid, x, y, z, type, FaceDirection::PosZ);

					BlockFace face = GetBlockFace(type, FaceDirection::PosZ, isTop, fluidLevel, neighbourFluidData);

					if (type == BlockType::Air || type == BlockType::Undefined)
						continue;

					Vec3f topLeft = face.vertices[3] + Vec3f(x, y, z);
					Vec3f topRight;
					Vec3f bottomRight;
					Vec3f bottomLeft;

					int endY, endX;
					for (endX = x + 1; endX < CHUNK_SIZE && grid(z, y, endX).type == type && grid(z, y, endX).fluidLevel == fluidLevel; endX++)
					{
						grid(z, y, endX).type = BlockType::Undefined;
					}

					for (endY = y + 1; endY < CHUNK_HEIGHT; endY++)
					{
						bool passed = true;
						for (int cx = x; cx < endX; cx++)
						{
							if (grid(z, endY, cx).type != type || grid(z, endY, cx).fluidLevel != fluidLevel)
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
					BlockType type = grid(x, z, y).type;
					bool isTop = false;
					uint8_t fluidLevel = grid(x, z, y).fluidLevel;
					if (y < CHUNK_HEIGHT - 1 && grid(x, z, y + 1).type != type) isTop = true;

					NeighbourFluidData neighbourFluidData = GetNeighbourFluidData(grid, x, y, z, type, FaceDirection::NegX);
					
					BlockFace face = GetBlockFace(type, FaceDirection::NegX, isTop, fluidLevel, neighbourFluidData);
					if (type == BlockType::Air || type == BlockType::Undefined)
						continue;

					Vec3f topLeft = face.vertices[3] + Vec3f(x, y, z);
					Vec3f topRight;
					Vec3f bottomRight;
					Vec3f bottomLeft;

					int endY, endZ;
					for (endZ = z + 1; endZ < CHUNK_SIZE && grid(x, endZ, y).type == type && grid(x, endZ, y).fluidLevel == fluidLevel; endZ++)
					{
						grid(x, endZ, y).type = BlockType::Undefined;
					}

					for (endY = y + 1; endY < CHUNK_HEIGHT; endY++)
					{
						bool passed = true;
						for (int cz = z; cz < endZ; cz++)
						{
							if (grid(x, cz, endY).type != type || grid(x, cz, endY).fluidLevel != fluidLevel)
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
					BlockType type = grid(y, z, x).type;
					bool isTop = false;
					uint8_t fluidLevel = grid(y, z, x).fluidLevel;
					if (y < CHUNK_HEIGHT - 1 && grid(y + 1, z, x).type != type) isTop = true;
					
					NeighbourFluidData neighbourFluidData = GetNeighbourFluidData(grid, x, y, z, type, FaceDirection::NegY);
					
					BlockFace face = GetBlockFace(type, FaceDirection::NegY, isTop, fluidLevel, neighbourFluidData);

					if (type == BlockType::Air || type == BlockType::Undefined)
						continue;

					Vec3f topLeft = face.vertices[1] + Vec3f(x, y, z);
					Vec3f topRight;
					Vec3f bottomRight;
					Vec3f bottomLeft;

					int endZ, endX;
					for (endX = x + 1; endX < CHUNK_SIZE && grid(y, z, endX).type == type && grid(y, z, endX).fluidLevel == fluidLevel; endX++)
					{
						grid(y, z, endX).type = BlockType::Undefined;
					}

					for (endZ = z + 1; endZ < CHUNK_SIZE; endZ++)
					{
						bool passed = true;
						for (int cx = x; cx < endX; cx++)
						{
							if (grid(y, endZ, cx).type != type || grid(y, endZ, cx).fluidLevel != fluidLevel)
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
					BlockType type = grid(z, x, y).type;
					bool isTop = false;
					uint8_t fluidLevel = grid(z, x, y).fluidLevel;
					if (y < CHUNK_HEIGHT - 1 && grid(z, x, y + 1).type != type) isTop = true;
					
					NeighbourFluidData neighbourFluidData = GetNeighbourFluidData(grid, x, y, z, type, FaceDirection::NegZ);
					
					BlockFace face = GetBlockFace(type, FaceDirection::NegZ, isTop, fluidLevel, neighbourFluidData);

					if (type == BlockType::Air || type == BlockType::Undefined)
						continue;

					Vec3f topLeft = face.vertices[0] + Vec3f(x, y, z);
					Vec3f topRight;
					Vec3f bottomRight;
					Vec3f bottomLeft;

					int endX, endY;
					for (endY = y + 1; endY < CHUNK_HEIGHT && grid(z, x, endY).type == type && grid(z, x, endY).fluidLevel == fluidLevel; endY++)
					{
						grid(z, x, endY).type = BlockType::Undefined;
					}

					for (endX = x + 1; endX < CHUNK_SIZE; endX++)
					{
						bool passed = true;
						for (int cy = y; cy < endY; cy++)
						{
							if (grid(z, endX, cy).type != type || grid(z, endX, cy).fluidLevel != fluidLevel)
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
		int idx = static_cast<int>(mesh.vertices.size());

		for (int i = 0; i < 4; i++)
		{
			Vertex v;
			v.position = face.vertices[i];
			v.normal = face.normal;

			v.u = face.uvs[i].x * uWraps;
			v.v = face.uvs[i].y * vWraps;

			TileType tileType = GetTileTypeFromDirection(type, dir);
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
		int startIndex = static_cast<int>(mesh.vertices.size());

		for (int i = 0; i < 4; i++)
		{
			Vertex v;
			v.position = Vec3f(
				face.vertices[i].x + blockPos.x,
				face.vertices[i].y + blockPos.y,
				face.vertices[i].z + blockPos.z
			);
			v.normal = face.normal;

			v.u = face.uvs[i].x;
			v.v = face.uvs[i].y;

			TileType tileType = GetTileTypeFromDirection(type, dir);
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

	NeighbourFluidData World::GetNeighbourFluidData(Mat3D<Block>& grid, int x, int y, int z, BlockType type, FaceDirection faceDirection) const
	{
		Block posXBlock, negXBlock, posZBlock, negZBlock;
		Block posXnegZBlock, posXposZBlock, negXnegZBlock, negXposZBlock;

		if (faceDirection == FaceDirection::PosX)
		{
			if (x < CHUNK_SIZE - 1) posXBlock = grid(x + 1, y, z);
			if (x > 0) negXBlock = grid(x - 1, y, z);
			if (z < CHUNK_SIZE - 1) posZBlock = grid(x, y, z + 1);
			if (z > 0) negZBlock = grid(x, y, z - 1);
			if (x < CHUNK_SIZE - 1 && z > 0) posXnegZBlock = grid(x + 1, y, z - 1);
			if (x < CHUNK_SIZE - 1 && z < CHUNK_SIZE - 1) posXposZBlock = grid(x + 1, y, z + 1);
			if (x > 0 && z > 0) negXnegZBlock = grid(x - 1, y, z - 1);
			if (x > 0 && z < CHUNK_SIZE - 1) negXposZBlock = grid(x - 1, y, z + 1);
		}
		else if (faceDirection == FaceDirection::NegX)
		{
			if (x < CHUNK_SIZE - 1) posXBlock = grid(x + 1, z, y);
			if (x > 0) negXBlock = grid(x - 1, z, y);
			if (z < CHUNK_SIZE - 1) posZBlock = grid(x, z + 1, y);
			if (z > 0) negZBlock = grid(x, z - 1, y);
			if (x < CHUNK_SIZE - 1 && z > 0) posXnegZBlock = grid(x + 1, z - 1, y);
			if (x < CHUNK_SIZE - 1 && z < CHUNK_SIZE - 1) posXposZBlock = grid(x + 1, z + 1, y);
			if (x > 0 && z > 0) negXnegZBlock = grid(x - 1, z - 1, y);
			if (x > 0 && z < CHUNK_SIZE - 1) negXposZBlock = grid(x - 1, z + 1, y);
		}
		else if (faceDirection == FaceDirection::PosY)
		{
			if (x < CHUNK_SIZE - 1) posXBlock = grid(y, x + 1, z);
			if (x > 0) negXBlock = grid(y, x - 1, z);
			if (z < CHUNK_SIZE - 1) posZBlock = grid(y, x, z + 1);
			if (z > 0) negZBlock = grid(y, x, z - 1);
			if (x < CHUNK_SIZE - 1 && z > 0) posXnegZBlock = grid(y, x + 1, z - 1);
			if (x < CHUNK_SIZE - 1 && z < CHUNK_SIZE - 1) posXposZBlock = grid(y, x + 1, z + 1);
			if (x > 0 && z > 0) negXnegZBlock = grid(y, x - 1, z - 1);
			if (x > 0 && z < CHUNK_SIZE - 1) negXposZBlock = grid(y, x - 1, z + 1);
		}
		else if (faceDirection == FaceDirection::NegY)
		{
			if (x < CHUNK_SIZE - 1) posXBlock = grid(y, z, x + 1);
			if (x > 0) negXBlock = grid(y, z, x - 1);
			if (z < CHUNK_SIZE - 1) posZBlock = grid(y, z + 1, x);
			if (z > 0) negZBlock = grid(y, z - 1, x);
			if (x < CHUNK_SIZE - 1 && z > 0) posXnegZBlock = grid(y, z - 1, x + 1);
			if (x < CHUNK_SIZE - 1 && z < CHUNK_SIZE - 1) posXposZBlock = grid(y, z + 1, x + 1);
			if (x > 0 && z > 0) negXnegZBlock = grid(y, z - 1, x - 1);
			if (x > 0 && z < CHUNK_SIZE - 1) negXposZBlock = grid(y, z + 1, x - 1);
		}
		else if (faceDirection == FaceDirection::PosZ)
		{
			if (x < CHUNK_SIZE - 1) posXBlock = grid(z, y, x + 1);
			if (x > 0) negXBlock = grid(z, y, x - 1);
			if (z < CHUNK_SIZE - 1) posZBlock = grid(z + 1, y, x);
			if (z > 0) negZBlock = grid(z - 1, y, x);
			if (x < CHUNK_SIZE - 1 && z > 0) posXnegZBlock = grid(z - 1, y, x + 1);
			if (x < CHUNK_SIZE - 1 && z < CHUNK_SIZE - 1) posXposZBlock = grid(z + 1, y, x + 1);
			if (x > 0 && z > 0) negXnegZBlock = grid(z - 1, y, x - 1);
			if (x > 0 && z < CHUNK_SIZE - 1) negXposZBlock = grid(z + 1, y, x - 1);

		}
		else if (faceDirection == FaceDirection::NegZ)
		{
			if (x < CHUNK_SIZE - 1) posXBlock = grid(z, x + 1, y);
			if (x > 0) negXBlock = grid(z, x - 1, y);
			if (z < CHUNK_SIZE - 1) posZBlock = grid(z + 1, x, y);
			if (z > 0) negZBlock = grid(z - 1, x, y);
			if (x < CHUNK_SIZE - 1 && z > 0) posXnegZBlock = grid(z - 1, x + 1, y);
			if (x < CHUNK_SIZE - 1 && z < CHUNK_SIZE - 1) posXposZBlock = grid(z + 1, x + 1, y);
			if (x > 0 && z > 0) negXnegZBlock = grid(z - 1, x - 1, y);
			if (x > 0 && z < CHUNK_SIZE - 1) negXposZBlock = grid(z + 1, x - 1, y);
		}

		NeighbourFluidData neighbourFluidData;

		if (posXBlock.type != BlockType::Air && posXBlock.type == type)
		{
			neighbourFluidData.posXFluidLevel = posXBlock.fluidLevel;
		}

		if (negXBlock.type != BlockType::Air && negXBlock.type == type)
		{
			neighbourFluidData.negXFluidLevel = negXBlock.fluidLevel;
		}

		if (posZBlock.type != BlockType::Air && posZBlock.type == type)
		{
			neighbourFluidData.posZFluidLevel = posZBlock.fluidLevel;
		}

		if (negZBlock.type != BlockType::Air && negZBlock.type == type)
		{
			neighbourFluidData.negZFluidLevel = negZBlock.fluidLevel;
		}

		if (posXnegZBlock.type != BlockType::Air && posXnegZBlock.type == type)
		{
			neighbourFluidData.posXnegZFluidLevel = posXnegZBlock.fluidLevel;
		}

		if (posXposZBlock.type != BlockType::Air && posXposZBlock.type == type)
		{
			neighbourFluidData.posXposZFluidLevel = posXposZBlock.fluidLevel;
		}

		if (negXnegZBlock.type != BlockType::Air && negXnegZBlock.type == type)
		{
			neighbourFluidData.negXnegZFluidLevel = negXnegZBlock.fluidLevel;
		}

		if (negXposZBlock.type != BlockType::Air && negXposZBlock.type == type)
		{
			neighbourFluidData.negXposZFluidLevel = negXposZBlock.fluidLevel;
		}

		return neighbourFluidData;
	}

	void World::AddChunkToMeshingQueue(Chunk& chunk)
	{
		if (chunk.state == ChunkState::NeedsMeshing) return;
		chunk.state = ChunkState::NeedsMeshing;
		m_ChunkMeshingQueue.push(chunk.position);
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

			if (plane.normal.Dot(positiveVertex) + plane.d < 0.0f)
			{
				return false;
			}
		}
		return true;
	}

	inline bool World::IsFaceVisible(
		Block block,
		Vec3i neighborPos, 
		const Chunk& chunk,
		const Chunk* chunkPosX,
		const Chunk* chunkNegX, 
		const Chunk* chunkPosZ,
		const Chunk* chunkNegZ
	) const
	{
		Block neighborBlock;
		bool overflowed = false;

		if (neighborPos.x < 0)
		{
			neighborBlock = chunkNegX->blocks[CHUNK_SIZE - 1][neighborPos.y][neighborPos.z];
			overflowed = true;
		}
		if (neighborPos.x > CHUNK_SIZE - 1)
		{
			neighborBlock = chunkPosX->blocks[0][neighborPos.y][neighborPos.z];
			overflowed = true;
		}
		if (neighborPos.z < 0)
		{
			neighborBlock = chunkNegZ->blocks[neighborPos.x][neighborPos.y][CHUNK_SIZE - 1];
			overflowed = true;
		}
		if (neighborPos.z > CHUNK_SIZE - 1)
		{
			neighborBlock = chunkPosZ->blocks[neighborPos.x][neighborPos.y][0];
			overflowed = true;
		}
		if (neighborPos.y < 0) return true;
		if (neighborPos.y >= CHUNK_HEIGHT) return true;

		if (!overflowed)
			neighborBlock = chunk.blocks[neighborPos.x][neighborPos.y][neighborPos.z];

		if (block.IsTransparent() && neighborBlock.type == block.type) return false;
		return neighborBlock.IsTransparent() || neighborBlock.IsCutoffTransparent();
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

}
