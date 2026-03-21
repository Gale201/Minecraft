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
		if (m_InventoryOpen)
		{
			m_Inventory.Update(m_InputManager);
			
			ItemStack holdingStack = m_Inventory.GetItemToRemove();
			if (holdingStack.size > 0)
			{
				Vec3f spawnPos = GetHeadPosition() + m_Camera.GetForward() * 0.5f;
				float rotY = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * PI;
				world.AddItem(PhysicsBody(spawnPos), holdingStack, rotY, m_Camera.GetForward() * 30.0f);
				world.GetItems()[world.GetItemIndex(world.GetCurrentItemID() - 1)].pickupCooldown = 120;
			}
		}
		else
			m_Inventory.UpdateHotbar(m_InputManager);
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

	void Player::Render(Renderer& renderer, const ItemRegistry& itemRegistry)
	{
		
	}

	void Player::RenderUI(Renderer& renderer, const ItemRegistry& itemRegistry, const FontLoader& fontLoader)
	{
		m_Inventory.RenderHotbar(renderer, itemRegistry, fontLoader);
		if (m_InventoryOpen)
			m_Inventory.Render(renderer, itemRegistry, fontLoader);
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

					if (item.pickupCooldown == 0 && item.body && (item.body->GetCenter() - m_PhysicsBody->GetCenter()).length() < m_ItemPickupDistance && m_Inventory.CanAddItemStack(item.stack))
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
			item.body->position += direction * 15.f * deltaTime;

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
		m_Inventory.GetSelectedHotbarSlot().size--;
	}
	
	void Player::HandleInput()
	{
		// Inventory
		if (m_InputManager.IsKeyJustPressed(m_InputBindings.GetKeyForAction(InputAction::OpenInventory)))
			m_InventoryOpen = !m_InventoryOpen;

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

		if (m_GameMode.CanInstantBreakBlocks() && m_InputManager.IsMouseButtonJustPressed(m_InputBindings.GetMouseButtonForAction(InputAction::BreakBlock)))
			m_RequestedBlockBreakAction = true;

		if (m_InputManager.IsMouseButtonJustPressed(m_InputBindings.GetMouseButtonForAction(InputAction::PlaceBlock)))
			m_RequestedBlockPlaceAction = true;

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

	void Player::SyncCamera(float alpha)
	{
		Vec3f interpolated = m_PhysicsBody->GetPreviousPosition() * (1.0f - alpha) + m_PhysicsBody->position * alpha;

		m_Camera.position = interpolated + GetRelativeCameraPos();
		m_Camera.pitch = m_HeadRotation.x;
		m_Camera.yaw = m_HeadRotation.y;
		m_Camera.roll = m_HeadRotation.z;
	}

	void Player::UpdateCameraAspectRatio(float ratio)
	{
		m_Camera.aspect = ratio;
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
		return Block(ItemRegistry::GetBlockTypeFromItemType(stack.type));
	}

}