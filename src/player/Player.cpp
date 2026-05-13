#include "Player.h"

#include <algorithm>

namespace Minecraft
{
	Player::Player(const Core::InputManager& inputManager, PhysicsSystem& physicsSystem, InputBindings inputBindings, CameraConfig cameraConfig) :
		m_InputManager(inputManager),
		m_PhysicsSystem(physicsSystem),
		m_InputBindings(inputBindings),
		m_Camera()
	{
		SetCameraConfig(cameraConfig);
	}

	void Player::Update(float deltaTime, World& world)
	{
		HandleInput();

		UpdateInventory(deltaTime, world);

		HandleBlockInteractions(world);

		UpdateAnimations(deltaTime);

		UpdateFirstPersonModel();

		// Sprinting FOV effect
		if (!m_InventoryOpen &&
			m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::Sprint)) &&
			m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::MoveForward)) &&
			m_PhysicsBody->collidingX == 0 &&
			m_PhysicsBody->collidingZ == 0)
			m_Camera.fovY = std::fminf(m_Camera.fovY + 0.04f, m_CameraSprintFovY);
		else
			m_Camera.fovY = std::fmaxf(m_Camera.fovY - 0.04f, m_CameraFovY);

		m_PlaceBlockTimer += deltaTime;
		m_BreakBlockTimer += deltaTime;

		m_JustPlacedBlock = false;
		m_JustBrokeBlock = false;

		m_DropItemStack = false;
	}

	void Player::UpdatePhysics(float deltaTime)
	{
		if (m_PhysicsBody->position.y < -100.0f)
			m_PhysicsBody->position.y = 70.0f;

		if (m_InventoryOpen)
			return;

		HandleMovementInput();

		if (m_CancelSprint && (m_PhysicsBody->isGrounded || m_Flying))
		{
			m_Sprinting = false;
			m_CancelSprint = false;
		}
		else if (m_StartSprint && (m_PhysicsBody->isGrounded || m_Flying))
		{
			m_Sprinting = true;
			m_StartSprint = false;
		}
	}

	void Player::Render(Renderer& renderer, const ItemRegistry& itemRegistry, int seaLevel)
	{
		if (m_WaterScreenTexture.GetWidth() != renderer.GetWidth() ||
			m_WaterScreenTexture.GetHeight() != renderer.GetHeight())
		{
			CreateWaterScreenTexture(renderer.GetWidth(), renderer.GetHeight());
		}

		if (IsInWater() && GetHeadPosition().y - 0.9f < seaLevel)
			renderer.DrawTexture(0, 0, m_WaterScreenTexture, true);

		renderer.DrawTexture(
			renderer.GetWidth() / 2 - m_CrosshairTexture->GetWidth() / 2,
			renderer.GetHeight() / 2 - m_CrosshairTexture->GetHeight() / 2,
			*m_CrosshairTexture, true
		);

		// Debugging
		Transform t1;
		t1.SetPosition(m_PhysicsBody->position);
		t1.SetScale(m_PhysicsBody->size);
		//renderer.DrawCubeWireframe(t1, 0xAAFF0000, m_Camera, 4, true, false);
	}

	void Player::RenderUI(Renderer& renderer, const ItemRegistry& itemRegistry, const FontLoader& fontLoader)
	{
		Camera camera;
		camera.fovY = 1.0f;
		camera.aspect = static_cast<float>(renderer.GetWidth()) / static_cast<float>(renderer.GetHeight());
		camera.nearPlane = 0.1f;
		camera.farPlane = 100.0f;

		if (m_Inventory.GetSelectedHotbarSlot().size <= 0)
			m_Model->RenderPart(renderer, camera, "RightArmUI");
		else
		{
			ItemDefinition itemDef = itemRegistry.GetItemDefinition(m_Inventory.GetSelectedHotbarSlot().type);
			AtlasTileInfo tileInfo = itemRegistry.GetAtlasTileInfoForItem(itemDef.type);
			
			Transform transform;
			if (itemRegistry.IsItemFullBlock(itemDef.type))
			{
				EntityModelPart* rightArmUI = m_Model->GetPart("RightArmUI");
				transform.SetPosition(rightArmUI->position + Vec3f(-0.016f, 0.01f, -0.05f));
				transform.SetRotation(rightArmUI->rotation + Vec3f(-2.0f + 0.1f, -3.7f - 0.8f, 0.0f));
				transform.SetScale(Vec3f(
					rightArmUI->scale.x * 1.5f,
					rightArmUI->scale.y * 1.5f,
					rightArmUI->scale.z * 1.5f
				));
			}
			else
			{
				EntityModelPart* rightArmUI = m_Model->GetPart("RightArmUI");
				Vec3f rot = m_FirstPersonRightArmTransform.GetRotation();
				transform.SetPosition(rightArmUI->position + Vec3f(-0.075f, 0.03f, -0.1f));
				transform.SetRotation(rightArmUI->rotation + Vec3f(-1.4f - rot.x, -rot.y, 1.4f - rot.z));
				transform.SetScale(Vec3f(
					rightArmUI->scale.x * 1.5f,
					rightArmUI->scale.y * 1.5f,
					rightArmUI->scale.z * 1.5f
				));
			}
			renderer.DrawMesh(*itemDef.mesh, transform, *itemDef.texture, camera, tileInfo);
		}

		m_Inventory.RenderHotbar(renderer, itemRegistry, fontLoader);
		if (m_GameMode.HasHealth())
			m_Inventory.RenderHotbarIcons(renderer, m_Health, m_Armor);

		if (m_InventoryOpen)
			m_Inventory.Render(renderer, itemRegistry, fontLoader);
	}

	void Player::UpdateInventory(float deltaTime, World& world)
	{
		if (m_InventoryOpen)
			m_Inventory.Update(deltaTime, m_InputManager);
		else
			m_Inventory.UpdateHotbar(m_InputManager);

		int slotX, slotY;
		if (m_InventoryOpen)
		{
			int mouseX = m_InputManager.GetMouseX();
			int mouseY = m_InputManager.GetMouseY();
			const auto slotCoords = m_Inventory.GetSlotCoordinatesFromScreenPosition(mouseX, mouseY);
			slotX = slotCoords.first;
			slotY = slotCoords.second;
		}
		else
		{
			slotX = m_Inventory.GetSelectedHotbarSlotIndex();
			slotY = 0;
		}

		if (m_DropItemStack)
		{
			m_Inventory.DropItemStack(slotX, slotY);
		}
		else if (m_StartedDroppingItem)
		{
			m_DropItemTimer += deltaTime;
			if (m_DropItemTimer >= m_DropItemInterval)
			{
				m_DropItemTimer = 0.0f;
				m_Inventory.DropOneItem(slotX, slotY);
			}
		}

		ItemStack itemStackToRemove = m_Inventory.GetItemToRemove();
		if (itemStackToRemove.size > 0)
		{
			Vec3f spawnPos = GetHeadPosition() + m_Camera.GetForward() * 0.5f;
			float rotY = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * PI;
			world.AddItem(PhysicsBody(spawnPos), itemStackToRemove, rotY, m_Camera.GetForward() * 30.0f);
			world.GetItems()[world.GetItemIndex(world.GetCurrentItemID() - 1)].pickupCooldown = world.GetItemPickupCooldown();
		}
	}

	void Player::HandleBlockInteractions(World& world)
	{
		if (m_InputManager.IsMouseButtonJustReleased(m_InputBindings.GetMouseButtonForAction(InputAction::BreakBlock)))
			m_StartedBreakingBlock = false;

		// Picking blocks
		WorldRaycastResult raycast;
		if (!m_InventoryOpen &&
			!m_GameMode.PlacingBlocksConsumesItems() &&
			m_Inventory.GetSelectedHotbarSlot().size == 0 &&
			world.Raycast(m_Camera.position, m_Camera.GetForward(), m_MaxBreakBlockDistance, raycast) &&
			m_InputManager.IsMouseButtonJustPressed(m_InputBindings.GetMouseButtonForAction(InputAction::PickBlock)))
		{
			m_Inventory.SetSlot(m_Inventory.GetSelectedHotbarSlotIndex(), { ItemRegistry::GetItemTypeFromBlockType(raycast.blockType), 1 });
		}

		if (m_GameMode.CanInstantBreakBlocks())
			return;

		// Breaking blocks
		if (
			m_InputManager.IsMouseButtonDown(m_InputBindings.GetMouseButtonForAction(InputAction::BreakBlock)) &&
			world.Raycast(m_Camera.position, m_Camera.GetForward(), m_MaxBreakBlockDistance, raycast)
			)
		{
			if (m_LastBlockBreakingPos != raycast.blockPosition)
				m_StartedBreakingBlock = false;
			else if (!m_StartedBreakingBlock)
			{
				m_StartedBreakingBlock = true;
				m_IsPunching = false;
				m_BlockBreakingStartTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
			}

			if (m_LastBlockBreakingPos == raycast.blockPosition)
			{
				Block block = world.GetBlock(raycast.blockPosition);
				float breakTime = block.GetBreakTime();
				if (block.IsBreakable())
				{
					long long currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
					float progress = (currentTime - m_BlockBreakingStartTime) / (breakTime * 1000.0f);
					world.SetBlockBreakingProgress(block, raycast.blockPosition, progress);
					if (progress >= 1.0f)
					{
						m_RequestedBlockBreakAction = true;
						m_StartedBreakingBlock = false;
					}
				}
				else
					m_StartedBreakingBlock = false;
			}

			m_LastBlockBreakingPos = raycast.blockPosition;
		}
		else
			m_StartedBreakingBlock = false;
	}

	void Player::UpdateAnimations(float deltaTime)
	{
		// Breaking animation
		if (!m_InventoryOpen && m_StartedBreakingBlock)
		{
			m_StartedBreakingBlockAnimation = true;
			m_StartedPunchingAnimation = false;
			m_StartedPlacingBlockAnimation = false;
		}

		if (m_StartedBreakingBlockAnimation)
			m_BreakingBlockAnimation.Update(deltaTime);

		if (!m_StartedBreakingBlock && m_BreakingBlockAnimation.JustFinishedCycle())
		{
			m_StartedBreakingBlockAnimation = false;
			m_BreakingBlockAnimation.Reset();
		}

		// Placing animation
		if (!m_InventoryOpen && m_JustPlacedBlock)
		{
			m_PlacingBlockAnimation.Reset();
			m_StartedPlacingBlockAnimation = true;
			m_StartedPunchingAnimation = false;
			m_StartedBreakingBlockAnimation = false;
		}

		if (m_StartedPlacingBlockAnimation)
			m_PlacingBlockAnimation.Update(deltaTime);

		if (m_PlacingBlockAnimation.JustFinishedCycle())
		{
			m_StartedPlacingBlockAnimation = false;
			m_PlacingBlockAnimation.Reset();
		}

		// Punching animation
		if (!m_InventoryOpen && m_IsPunching)
		{
			m_StartedPunchingAnimation = true;
			m_StartedBreakingBlockAnimation = false;
			m_StartedPlacingBlockAnimation = false;
		}

		if (m_StartedPunchingAnimation)
			m_PunchingAnimation.Update(deltaTime);

		if (m_PunchingAnimation.JustFinishedCycle())
		{
			m_StartedPunchingAnimation = false;
			m_PunchingAnimation.Reset();
		}
	}

	void Player::HandleItemPickup(World& world, PhysicsSystem& physicsSystem, float deltaTime)
	{
		Vec3i chunkPos = world.GetChunkPosition(Vec3i::floor(m_PhysicsBody->GetCenter()));

		auto& items = world.GetItems();
		auto& itemsByChunk = world.GetItemsByChunk();

		for (int x = -1; x <= 1; x++)
			for (int z = -1; z <= 1; z++)
			{
				Vec3i neighbor = chunkPos + Vec3i(x, 0, z);

				auto it = itemsByChunk.find(neighbor);
				if (it == itemsByChunk.end())
					continue;

				const auto& indices = it->second;

				for (size_t index : indices)
				{
					Item& item = items[index];

					if (std::find(m_ItemsToRemove.begin(), m_ItemsToRemove.end(), item.id) != m_ItemsToRemove.end())
						continue;

					if (item.pickupCooldown <= 0.0f && item.body && (item.body->GetCenter() - m_PhysicsBody->GetCenter()).length() < m_ItemPickupDistance && m_Inventory.CanAddItemStack(item.stack))
					{
						m_ItemsToRemove.push_back(item.id);
						physicsSystem.RemoveBody(item.body);
					}
				}
			}

		for (auto it = m_ItemsToRemove.begin(); it != m_ItemsToRemove.end(); )
		{
			ItemID id = *it;

			size_t index = world.GetItemIndex(id);
			Item& item = items[index];

			Vec3f direction = (m_PhysicsBody->GetCenter() - item.body->GetCenter()).normalized();
			item.body->position += direction * 10.f * deltaTime;

			if ((item.body->GetCenter() - m_PhysicsBody->GetCenter()).length() <= 0.5f)
			{
				it = m_ItemsToRemove.erase(it);
				ItemStack leftover = m_Inventory.AddItemStack(item.stack);
				if (leftover.size == 0)
					world.RemoveItem(item.id);
				else
				{
					size_t index = world.GetItemIndex(item.id);
					world.GetItems()[index].stack.size = leftover.size;
				}
			}
			else
				++it;
		}
	}

	void Player::RemoveItemAfterPlacing()
	{
		if (m_GameMode.PlacingBlocksConsumesItems())
			m_Inventory.GetSelectedHotbarSlot().size--;

		m_JustPlacedBlock = true;
	}
	
	void Player::SyncCamera(float alpha)
	{
		Vec3f interpolated = m_PhysicsBody->GetPreviousPosition() * (1.0f - alpha) + m_PhysicsBody->position * alpha;

		m_Camera.position = interpolated + GetRelativeCameraPos();
		m_Camera.pitch = m_HeadRotation.x;
		m_Camera.yaw = m_HeadRotation.y;
		m_Camera.roll = m_HeadRotation.z;
	}

	bool Player::IsInBlock(Vec3i blockPos) const
	{
		for (int i = -1; i <= 1; i++)
			for (int j = -1; j <= 1; j++)
			{
				Vec3f feetPos = GetFeetPosition() + Vec3f(i * m_Size.x / 2.0f, 0.0f, j * m_Size.z / 2.0f);
				Vec3f headPos = GetHeadPosition() + Vec3f(i * m_Size.x / 2.0f, 0.0f, j * m_Size.z / 2.0f);
				if (Vec3i::floor(feetPos) == blockPos || Vec3i::floor(headPos) == blockPos) return true;
			}
		return false;
	}

	bool Player::RequestedBlockBreakAction() const
	{
		return m_RequestedBlockBreakAction;
	}

	bool Player::RequestedBlockPlaceAction() const
	{
		return m_RequestedBlockPlaceAction;
	}

	Block Player::GetRequestedBlockToPlace() const
	{
		const ItemStack& stack = m_Inventory.GetSelectedHotbarSlot();
		if (stack.size == 0 || !ItemRegistry::IsItemPlacable(stack.type))
			return Block(BlockType::Undefined);
		if (ItemRegistry::GetBlockTypeFromItemType(stack.type) == BlockType::Gravel)
			return Block(BlockType::Water);
		return Block(ItemRegistry::GetBlockTypeFromItemType(stack.type));
	}

	void Player::SetModel(EntityModel& model)
	{
		m_Model = std::make_unique<EntityModel>(model);
		m_Model->SetTexture(*m_SkinTexture);
		EntityModelPart rightArmUI;
		EntityModelPart* rightArm = m_Model->GetPart("RightArm");
		rightArmUI.position = rightArm->position;
		rightArmUI.rotation = rightArm->rotation;
		rightArmUI.scale = rightArm->scale;
		rightArmUI.mesh = rightArm->mesh;
		m_Model->AddPart("RightArmUI", rightArmUI);

		UpdateFirstPersonModel();
		m_Inventory.SetPlayerModel(model);
	}

	/* Private functions */

	void Player::HandleInput()
	{
		// Inventory
		if (m_InputManager.IsKeyJustPressed(m_InputBindings.GetKeyForAction(InputAction::OpenInventory)))
			m_InventoryOpen = !m_InventoryOpen;

		if (m_InputManager.IsKeyJustPressed(m_InputBindings.GetKeyForAction(InputAction::DropItem)))
		{
			m_StartedDroppingItem = true;
			m_DropItemTimer = 0.1f;
		}
		else if (m_InputManager.IsKeyJustReleased(m_InputBindings.GetKeyForAction(InputAction::DropItem)))
		{
			m_StartedDroppingItem = false;
			m_DropItemTimer = 0.0f;
		}

		if (m_InputManager.IsKeyJustPressed(m_InputBindings.GetKeyForAction(InputAction::DropItem)) &&
			m_InputManager.IsKeyDown(Key::LeftCtrl))
		{
			m_DropItemStack = true;
			m_StartedDroppingItem = false;
		}

		if (m_InventoryOpen)
			return;

		if (m_GameMode.CanPlayerFly() && m_InputManager.IsKeyJustDoublePressed(m_InputBindings.GetKeyForAction(InputAction::Jump)))
		{
			m_Flying = !m_Flying;
			m_PhysicsBody->applyGravity = !m_PhysicsBody->applyGravity;
		}

		// Placing/Destroying blocks
		m_RequestedBlockBreakAction = false;
		m_RequestedBlockPlaceAction = false;

		if (
			m_GameMode.CanInstantBreakBlocks() &&
			m_InputManager.IsMouseButtonDown(m_InputBindings.GetMouseButtonForAction(InputAction::BreakBlock)) &&
			m_BreakBlockTimer > m_BlockBreakingInterval
			)
		{
			m_RequestedBlockBreakAction = true;
			m_BreakBlockTimer = 0.0f;
		}

		if (m_InputManager.IsMouseButtonJustPressed(m_InputBindings.GetMouseButtonForAction(InputAction::PlaceBlock)))
		{
			m_RequestedBlockPlaceAction = true;
			m_PlaceBlockTimer = 0.0f;
		}

		if (m_InputManager.IsMouseButtonDown(m_InputBindings.GetMouseButtonForAction(InputAction::PlaceBlock)) &&
			m_PlaceBlockTimer > m_BlockPlacingInterval)
		{
			m_RequestedBlockPlaceAction = true;
			m_PlaceBlockTimer = 0.0f;
		}

		if (m_InputManager.IsMouseButtonJustReleased(m_InputBindings.GetMouseButtonForAction(InputAction::BreakBlock)))
			m_StartedBreakingBlock = false;

		// Punching
		if (m_InputManager.IsMouseButtonJustPressed(m_InputBindings.GetMouseButtonForAction(InputAction::BreakBlock)) && !m_StartedBreakingBlock)
			m_IsPunching = true;
		else
			m_IsPunching = false;

		// Sprinting & sneaking
		if (m_InputManager.IsKeyJustPressed(m_InputBindings.GetKeyForAction(InputAction::Sprint)))
		{
			m_StartSprint = true;
			m_CancelSprint = false;
		}
		if (m_InputManager.IsKeyJustReleased(m_InputBindings.GetKeyForAction(InputAction::Sprint)))
		{
			m_CancelSprint = true;
			m_StartSprint = false;
		}
		m_Sneaking = false;
		if (!m_Flying && m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::Sneak)))
			m_Sneaking = true;

		if (m_Sneaking)
			m_PhysicsBody->size.y = std::max(m_PhysicsBody->size.y - 0.05f, m_SneakHeight);
		else
			m_PhysicsBody->size = { m_Size.x, std::min(m_PhysicsBody->size.y + 0.05f, m_Size.y), m_Size.z };

		// Look
		m_HeadRotation.y += m_InputManager.GetMouseMotionX() * m_Sensitivity;
		if (m_HeadRotation.y < -2.0f * PI) m_HeadRotation.y += 2.0f * PI;
		if (m_HeadRotation.y > 2.0f * PI) m_HeadRotation.y -= 2.0f * PI;
		m_HeadRotation.x += m_InputManager.GetMouseMotionY() * m_Sensitivity;
		m_HeadRotation.x = std::clamp(m_HeadRotation.x, -m_PitchLimit, m_PitchLimit);
	}

	void Player::HandleMovementInput()
	{
		Vec3f force;

		// Movement
		Vec3f upVector{ 0.0f, 1.0f, 0.0f };
		Vec3f forwardVector = -upVector.Cross(m_Camera.GetRight()).normalized();

		if (!m_GameMode.CanPlayerFly()) m_Flying = false;

		if (m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::MoveForward)))
			force += forwardVector;

		if (m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::MoveBackward)))
			force -= forwardVector;

		if (m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::MoveRight)))
			force += m_Camera.GetRight();

		if (m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::MoveLeft)))
			force -= m_Camera.GetRight();

		if (m_Flying && m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::MoveDown)))
			force -= upVector;
		else if (m_PhysicsBody->inWater && m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::MoveDown)))
			m_PhysicsBody->AddForce({ 0.0f, -30.0f, 0.0f });

		if (!m_PhysicsBody->inWater && !m_Flying && m_PhysicsBody->isGrounded && m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::Jump)))
			m_PhysicsBody->AddForce(m_JumpVelocity);
		else if (m_PhysicsBody->inWater && !m_Flying && m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::MoveUp)))
			m_PhysicsBody->AddForce({ 0.0f, 30.0f, 0.0f });
		else if (m_Flying && m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::MoveUp)))
			force += upVector;
	
		if (m_Sprinting && !m_PhysicsBody->inWater)
			force = force.normalized() * m_SprintSpeed;
		else
			force = force.normalized() * m_Speed;

		if (m_Flying)
			m_PhysicsBody->flyingFriction = 10.0f;
		else
			m_PhysicsBody->flyingFriction = 0.0f;

		float forceMultiplier = m_PhysicsBody->isGrounded ? 10.0f : 2.f;
		if (m_Flying)
			force = { force.x, force.y * 12.0f, force.z };
		if (m_PhysicsBody->inWater)
			forceMultiplier = 3.0f;
		if (m_Sneaking)
			forceMultiplier *= 0.4f;
		m_PhysicsBody->AddForce(force * forceMultiplier);
	}

	void Player::UpdateFirstPersonModel()
	{
		Transform rightArmTransform;
		if (m_StartedPunchingAnimation)
			rightArmTransform = m_PunchingAnimationFrames[m_PunchingAnimation.GetCurrentFrame()];
		else if (m_StartedBreakingBlockAnimation)
			rightArmTransform = m_BreakingBlockAnimationFrames[m_BreakingBlockAnimation.GetCurrentFrame()];
		else if (m_StartedPlacingBlockAnimation)
			rightArmTransform = m_PlacingBlockAnimationFrames[m_PlacingBlockAnimation.GetCurrentFrame()];
		else
			rightArmTransform = m_FirstPersonRightArmTransform;

		m_Model->GetPart("RightArmUI")->position = rightArmTransform.GetPosition() - m_Model->GetPosition();
		m_Model->GetPart("RightArmUI")->rotation = rightArmTransform.GetRotation() - m_Model->GetRotation();
		m_Model->GetPart("RightArmUI")->scale.x = rightArmTransform.GetScale().x / m_Model->GetScale().x;
		m_Model->GetPart("RightArmUI")->scale.y = rightArmTransform.GetScale().y / m_Model->GetScale().y;
		m_Model->GetPart("RightArmUI")->scale.z = rightArmTransform.GetScale().z / m_Model->GetScale().z;
	}

	void Player::CreateWaterScreenTexture(int width, int height)
	{
		m_WaterScreenTexture = Texture(width, height);
		for (int i = 0; i < width; i++)
			for (int j = 0; j < height; j++)
				m_WaterScreenTexture.SetPixel(i, j, 0x334444FF);
		m_WaterScreenTexture.SetDefaultColor(0x334444FF);
	}
}