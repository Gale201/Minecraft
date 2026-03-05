#pragma once
#include "../math/Math.h"
#include "../render/CameraController.h"
#include "../render/Camera.h"
#include "../world/Block.h"
#include "../physics/PhysicsBody.h"
#include "../game/GameMode.h"
#include "../inventory/Inventory.h"

#define DEFAULT_PLAYER_SPEED 6.0f
#define DEFAULT_PLAYER_SENSITIVITY 0.001f
#define DEFAULT_PITCH_LIMIT 1.55f

namespace Minecraft
{
	class Player
	{
	public:
		Player(const Core::InputManager& inputManager) : m_InputManager(inputManager)
		{
			m_PhysicsBody.size = m_Size;
		}
		Player(const Core::InputManager& inputManager, InputBindings inputBindings, CameraConfig cameraConfig);

		~Player() = default;

		void Update(float deltaTime);
		void SyncCamera();

		bool RequestedBlockBreakAction() const;
		bool RequestedBlockPlaceAction() const;
		Block GetRequestedBlockToPlace() const;

		bool IsInBlock(Vec3i block) const;
		bool IsInWater() const { return m_PhysicsBody.inWater; }

		/* Getters & Setters */
		const Camera& GetCamera() const { return m_Camera; }
		Vec3f GetPosition() const { return m_PhysicsBody.position; }
		Vec3f GetHeadRotation() const { return m_HeadRotation; }
		PhysicsBody& GetBody() { return m_PhysicsBody; }
		const GameMode& GetGameMode() const { return m_GameMode; }
		Vec3f GetRelativeCameraPos() const { return m_PhysicsBody.size * 0.5f + Vec3f(0.0f, m_PhysicsBody.size.y / 2.0f - 0.1f, 0.0f); }
		Vec3f GetFeetPosition() const { return m_PhysicsBody.position + Vec3f(m_Size.x / 2.0f, 0.0f, m_Size.z / 2.0f); }
		Vec3f GetHeadPosition() const { return m_PhysicsBody.position + GetRelativeCameraPos(); }
		float GetMaxBreakBlockDistance() const { return m_MaxBreakBlockDistance; }
		float GetMaxPlaceBlockDistance() const { return m_MaxPlaceBlockDistance; }

		void SetPosition(Vec3f position)
		{
			m_PhysicsBody.position = position;
			SyncCamera();
		}

		void SetCameraConfig(CameraConfig cameraConfig)
		{
			m_Camera.fovY = cameraConfig.aspectRatio;
			m_Camera.aspect = cameraConfig.fov;
			m_Camera.nearPlane = cameraConfig.nearPlane;
			m_Camera.farPlane = cameraConfig.farPlane;

			SyncCamera();
		}

		void SetHeadRotation(Vec3f rotation) { m_HeadRotation = rotation; }

		void SetInputBindings(InputBindings inputBindings) { m_InputBindings = inputBindings; }

	private:
		void HandleInput();

	private:
		Vec3f m_HeadRotation;
		Vec3f m_Size{ 0.6f, 1.6f, 0.6f };
		float m_Speed = DEFAULT_PLAYER_SPEED;
		float m_SprintSpeed = DEFAULT_PLAYER_SPEED * 1.4f;
		float m_Sensitivity = DEFAULT_PLAYER_SENSITIVITY;
		float m_PitchLimit = DEFAULT_PITCH_LIMIT;

		Camera m_Camera;
		InputBindings m_InputBindings;

		PhysicsBody m_PhysicsBody;

		Inventory m_Inventory;

		GameMode m_GameMode{ GameModeType::Creative };

		const Core::InputManager& m_InputManager;

		bool m_Flying = false;

		Vec3f m_JumpVelocity{ 0.0f, 520.0f, 0.0f };
		bool m_Sprinting = false;
		bool m_Sneaking = false;
		bool m_CancelSprint = false;
		bool m_StartSprint = false;

		float m_SneakHeight = 1.4f;

		float m_MaxBreakBlockDistance = 8.0f;
		float m_MaxPlaceBlockDistance = 8.0f;
		bool m_RequestedBlockBreakAction = false;
		bool m_RequestedBlockPlaceAction = false;
	};
}
