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
#include "../animations/FrameAnimation.h"
#include "../game/DebugOverlay.h"

#define DEFAULT_PLAYER_SPEED 5.0f
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
		void Update(float deltaTime, World& world, DebugOverlay& debugOverlay);
		void Render(Renderer& renderer, const ItemRegistry& itemRegistry, int seaLevel);
		void RenderUI(Renderer& renderer, const ItemRegistry& itemRegistry, const FontLoader& fontLoader);

		void UpdateInventory(float deltaTime, World& world);
		void HandleBlockInteractions(World& world);
		void UpdateAnimations(float deltaTime);

		void HandleItemPickup(World& world, PhysicsSystem& physicsSystem, float deltaTime);
		void RemoveItemAfterPlacing();

		void SyncCamera(float alpha = 1.0f);

		bool RequestedBlockBreakAction() const;
		bool RequestedBlockPlaceAction() const;
		Block GetRequestedBlockToPlace() const;

		bool IsInBlock(Vec3i block) const;
		bool IsInWater() const { return m_PhysicsBody->inWater; }
		bool IsInventoryOpen() const { return m_InventoryOpen; }
		bool IsSprinting() const { return m_Sprinting; }

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
			m_CameraFovY = cameraConfig.fov;
			m_CameraSprintFovY = cameraConfig.fov * 1.1f;

			SyncCamera();
		}

		void SetCameraFov(float fov) { m_Camera.fovY = fov; }

		void SetCameraAspectRatio(float ratio) { m_Camera.aspect = ratio; }

		void SetHeadRotation(Vec3f rotation) { m_HeadRotation = rotation; }

		void SetInputBindings(InputBindings inputBindings) { m_InputBindings = inputBindings; }

		void SetGameMode(GameMode mode) { m_GameMode = mode; }

		void SetJustBrokeBlock(bool value) { m_JustBrokeBlock = value; }

		void SetStartedBreakingBlock(bool value) { m_StartedBreakingBlock = value; }

		void SetTextures(Texture& crosshair, Texture& skinTexture) 
		{
			m_CrosshairTexture = &crosshair;
			m_SkinTexture = &skinTexture;
		}

		void SetModel(EntityModel& model);

	private:
		void HandleInput();
		void HandleMovementInput();
		void CreateWaterScreenTexture(int width, int height);
		void UpdateFirstPersonModel();

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

		bool m_IsPunching = false;

		bool m_DropItemStack = false;
		bool m_StartedDroppingItem = false;
		float m_DropItemTimer = 0.0f;
		float m_DropItemInterval = 0.1f;

		float m_CameraFovY = 1.0f;
		float m_CameraSprintFovY = 1.1f;

		bool m_InventoryOpen = false;

		float m_SneakHeight = 1.4f;

		float m_ItemPickupDistance = 1.5f;
		std::vector<ItemID> m_ItemsToRemove;

		float m_MaxBreakBlockDistance = 6.0f;
		float m_MaxPlaceBlockDistance = 6.0f;
		bool m_RequestedBlockBreakAction = false;
		bool m_RequestedBlockPlaceAction = false;

		bool m_JustPlacedBlock = false;
		bool m_JustBrokeBlock = false;

		bool m_StartedBreakingBlock = false;
		long long m_BlockBreakingStartTime = 0;
		Vec3i m_LastBlockBreakingPos;

		float m_BlockBreakingInterval = 0.2f;
		float m_BreakBlockTimer = 0.0f;
		float m_BlockPlacingInterval = 0.2f;
		float m_PlaceBlockTimer = 0.0f;

		int m_Health = 11;
		int m_Armor = 15;

		Texture* m_CrosshairTexture = nullptr;
		Texture* m_SkinTexture = nullptr;
		Texture m_WaterScreenTexture;
		Mesh m_WaterMesh;

		std::unique_ptr<EntityModel> m_Model = nullptr;

		Transform m_FirstPersonRightArmTransform{ 
			Vec3f(-0.084f, -0.10f, -0.125f), 
			Vec3f(2.0f, 3.7f, 0.0f), 
			Vec3f(1.f / 128.f) 
		};

		constexpr static int s_PunchingAnimationFrameCount = 12;
		constexpr static float s_PunchingAnimationFrameDuration = 0.02f;
		FrameAnimation m_PunchingAnimation{
			s_PunchingAnimationFrameCount,
			s_PunchingAnimationFrameDuration
		};
		std::array<Transform, s_PunchingAnimationFrameCount> m_PunchingAnimationFrames = 
		{
			Transform(Vec3f(-0.084f, -0.10f, -0.125f), Vec3f(2.0f, 3.7f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.090f, -0.11f, -0.115f), Vec3f(2.2f, 3.8f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.095f, -0.12f, -0.110f), Vec3f(2.4f, 3.9f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.080f, -0.10f, -0.140f), Vec3f(1.9f, 3.7f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.060f, -0.10f, -0.165f), Vec3f(1.6f, 3.5f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.040f, -0.11f, -0.185f), Vec3f(1.3f, 3.3f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.020f, -0.13f, -0.200f), Vec3f(1.1f, 3.2f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.035f, -0.15f, -0.175f), Vec3f(1.4f, 3.3f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.060f, -0.16f, -0.150f), Vec3f(1.7f, 3.5f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.075f, -0.14f, -0.135f), Vec3f(1.9f, 3.6f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.084f, -0.12f, -0.125f), Vec3f(2.0f, 3.7f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.084f, -0.10f, -0.125f), Vec3f(2.0f, 3.7f, 0.0f), Vec3f(1.f / 128.f)),
		};
		bool m_StartedPunchingAnimation = false;

		constexpr static int s_BreakingBlockAnimationFrameCount = 12;
		constexpr static float s_BreakingBlockAnimationFrameDuration = 0.02f;
		FrameAnimation m_BreakingBlockAnimation{
			s_BreakingBlockAnimationFrameCount,
			s_BreakingBlockAnimationFrameDuration
		};
		std::array<Transform, s_BreakingBlockAnimationFrameCount> m_BreakingBlockAnimationFrames =
		{
			Transform(Vec3f(-0.084f, -0.10f, -0.125f), Vec3f(2.0f, 3.7f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.070f, -0.08f, -0.135f), Vec3f(1.9f, 3.6f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.050f, -0.07f, -0.150f), Vec3f(1.7f, 3.5f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.040f, -0.06f, -0.145f), Vec3f(1.6f, 3.4f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.030f, -0.10f, -0.170f), Vec3f(1.4f, 3.2f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.020f, -0.14f, -0.190f), Vec3f(1.2f, 3.0f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.015f, -0.17f, -0.200f), Vec3f(1.1f, 2.9f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.025f, -0.15f, -0.180f), Vec3f(1.3f, 3.1f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.045f, -0.13f, -0.160f), Vec3f(1.6f, 3.3f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.060f, -0.12f, -0.145f), Vec3f(1.8f, 3.5f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.075f, -0.11f, -0.135f), Vec3f(1.9f, 3.6f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.084f, -0.10f, -0.125f), Vec3f(2.0f, 3.7f, 0.0f), Vec3f(1.f / 128.f))
		};
		bool m_StartedBreakingBlockAnimation = false;

		constexpr static int s_PlacingBlockAnimationFrameCount = 12;
		constexpr static float s_PlacingBlockAnimationFrameDuration = 0.02f;
		FrameAnimation m_PlacingBlockAnimation{
			s_PlacingBlockAnimationFrameCount,
			s_PlacingBlockAnimationFrameDuration
		};
		std::array<Transform, s_PlacingBlockAnimationFrameCount> m_PlacingBlockAnimationFrames =
		{
			Transform(Vec3f(-0.084f, -0.10f, -0.125f), Vec3f(2.0f, 3.7f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.090f, -0.11f, -0.115f), Vec3f(2.2f, 3.8f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.095f, -0.12f, -0.110f), Vec3f(2.4f, 3.9f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.080f, -0.10f, -0.140f), Vec3f(1.9f, 3.7f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.060f, -0.10f, -0.165f), Vec3f(1.6f, 3.5f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.040f, -0.11f, -0.185f), Vec3f(1.3f, 3.3f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.020f, -0.13f, -0.200f), Vec3f(1.1f, 3.2f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.035f, -0.15f, -0.175f), Vec3f(1.4f, 3.3f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.060f, -0.16f, -0.150f), Vec3f(1.7f, 3.5f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.075f, -0.14f, -0.135f), Vec3f(1.9f, 3.6f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.084f, -0.12f, -0.125f), Vec3f(2.0f, 3.7f, 0.0f), Vec3f(1.f / 128.f)),
			Transform(Vec3f(-0.084f, -0.10f, -0.125f), Vec3f(2.0f, 3.7f, 0.0f), Vec3f(1.f / 128.f)),
		};
		bool m_StartedPlacingBlockAnimation = false;
	};
}
