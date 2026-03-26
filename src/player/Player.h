#pragma once
#include "../math/Math.h"
#include "../render/CameraController.h"
#include "../render/Camera.h"
#include "../world/Block.h"
#include "../physics/PhysicsBody.h"
#include "../game/GameMode.h"
#include "../inventory/Inventory.h"
#include "../items/Item.h"
#include "../world/World.h"
#include "../physics/PhysicsSystem.h"

#define DEFAULT_PLAYER_SPEED 6.0f
#define DEFAULT_PLAYER_SENSITIVITY 0.001f
#define DEFAULT_PITCH_LIMIT 1.55f

namespace Minecraft
{
	class Player
	{
	public:
		Player(const Core::InputManager& inputManager, const PhysicsSystem& physicsSystem) :
			m_InputManager(inputManager), m_PhysicsSystem(physicsSystem)
		{
		}
		Player(const Core::InputManager& inputManager, PhysicsSystem& physicsSystem, InputBindings inputBindings, CameraConfig cameraConfig);

		~Player() = default;

		void UpdatePhysics(float deltaTime);
		void Update(float deltaTime, World& world);
		void Render(Renderer& renderer, const ItemRegistry& itemRegistry, int seaLevel);
		void RenderUI(Renderer& renderer, const ItemRegistry& itemRegistry, const FontLoader& fontLoader);

		void HandleItemPickup(World& world, PhysicsSystem& physicsSystem, float deltaTime);
		void RemoveItemAfterPlacing();

		void SyncCamera(float alpha = 1.0f);
		void UpdateCameraAspectRatio(float ratio);

		bool RequestedBlockBreakAction() const;
		bool RequestedBlockPlaceAction() const;
		Block GetRequestedBlockToPlace() const;

		bool IsInBlock(Vec3i block) const;
		bool IsInWater() const { return m_PhysicsBody->inWater; }
		bool IsInventoryOpen() const { return m_InventoryOpen; }

		/* Getters & Setters */
		const Camera& GetCamera() const { return m_Camera; }
		Vec3f GetPosition() const { return m_PhysicsBody->position; }
		Vec3f GetHeadRotation() const { return m_HeadRotation; }
		PhysicsBody& GetBody() { return *m_PhysicsBody; }
		const GameMode& GetGameMode() const { return m_GameMode; }
		Vec3f GetRelativeCameraPos() const { return m_PhysicsBody->size * 0.5f + Vec3f(0.0f, m_PhysicsBody->size.y / 2.0f - 0.1f, 0.0f); }
		Vec3f GetFeetPosition() const { return m_PhysicsBody->position + Vec3f(m_Size.x / 2.0f, 0.0f, m_Size.z / 2.0f); }
		Vec3f GetHeadPosition() const { return m_PhysicsBody->position + GetRelativeCameraPos(); }
		float GetMaxBreakBlockDistance() const { return m_MaxBreakBlockDistance; }
		float GetMaxPlaceBlockDistance() const { return m_MaxPlaceBlockDistance; }
		Vec3f GetInitialPosition() const { return m_InitialPosition; }
		Vec3f GetInitialSize() const { return m_Size; }
		Inventory& GetInventory() { return m_Inventory; }
		const Inventory& GetInventory() const { return m_Inventory; }
		int GetSelectedHotbarSlotIndex() const { return m_Inventory.GetSelectedHotbarSlotIndex(); }

		void SetPhysicsBodyPtr(PhysicsBody* body)
		{
			m_PhysicsBody = body;
			body->position = m_InitialPosition;
			body->size = m_Size;
		}

		void SetInitialPosition(Vec3f position)
		{
			m_InitialPosition = position;
		}

		void SetCameraConfig(CameraConfig cameraConfig)
		{
			m_Camera.fovY = cameraConfig.aspectRatio;
			m_Camera.aspect = cameraConfig.fov;
			m_Camera.nearPlane = cameraConfig.nearPlane;
			m_Camera.farPlane = cameraConfig.farPlane;

			SyncCamera();
		}

		void SetCameraFov(float fov) { m_Camera.fovY = fov; }

		void SetHeadRotation(Vec3f rotation) { m_HeadRotation = rotation; }

		void SetInputBindings(InputBindings inputBindings) { m_InputBindings = inputBindings; }

		void SetGameMode(GameMode mode) { m_GameMode = mode; }

		void SetTextures(Texture& crosshair) { m_CrosshairTexture = &crosshair; }

	private:
		void HandleInput();
		void HandleMovementInput();
		void CreateWaterScreenTexture(int width, int height);

	private:
		Vec3f m_HeadRotation;
		Vec3f m_Size{ 0.6f, 1.6f, 0.6f };
		Vec3f m_InitialPosition;
		float m_Speed = DEFAULT_PLAYER_SPEED;
		float m_SprintSpeed = DEFAULT_PLAYER_SPEED * 1.4f;
		float m_Sensitivity = DEFAULT_PLAYER_SENSITIVITY;
		float m_PitchLimit = DEFAULT_PITCH_LIMIT;

		Camera m_Camera;
		InputBindings m_InputBindings;

		PhysicsSystem m_PhysicsSystem;
		PhysicsBody* m_PhysicsBody = nullptr;

		Inventory m_Inventory;

		GameMode m_GameMode{ GameModeType::Creative };

		const Core::InputManager& m_InputManager;

		bool m_Flying = false;

		Vec3f m_JumpVelocity{ 0.0f, 400.0f, 0.0f };
		bool m_Sprinting = false;
		bool m_Sneaking = false;
		bool m_CancelSprint = false;
		bool m_StartSprint = false;

		bool m_InventoryOpen = false;

		float m_SneakHeight = 1.4f;

		float m_ItemPickupDistance = 2.5f;
		std::vector<ItemID> m_ItemsToRemove;

		float m_MaxBreakBlockDistance = 8.0f;
		float m_MaxPlaceBlockDistance = 8.0f;
		bool m_RequestedBlockBreakAction = false;
		bool m_RequestedBlockPlaceAction = false;

		bool m_StartedBreakingBlock = false;
		long long m_BlockBreakingStartTime = 0;
		Vec3i m_LastBlockBreakingPos;

		int m_Health = 11;
		int m_Armor = 15;

		Texture* m_CrosshairTexture;
		Texture m_WaterScreenTexture;
		Mesh m_WaterMesh;
	};
}
