#pragma once
#include "../math/Math.h"
#include "../render/CameraController.h"
#include "../render/Camera.h"
#include "../world/Block.h"

#define DEFAULT_PLAYER_SPEED 30.0f
#define DEFAULT_PLAYER_SENSITIVITY 0.001f
#define DEFAULT_PITCH_LIMIT 1.55f

namespace Minecraft
{
	class Player
	{
	public:
		Player(const Core::InputManager& inputManager) : m_InputManager(inputManager) {}
		Player(const Core::InputManager& inputManager, InputBindings inputBindings, CameraConfig cameraConfig);

		~Player() = default;

		void Update(float deltaTime);

		bool RequestedBlockBreakAction();
		bool RequestedBlockPlaceAction();
		Block GetRequestedBlockToPlace();

		/* Getters & Setters */
		const Camera& GetCamera() const { return m_Camera; }
		Vec3f GetPosition() const { return m_Position; }
		Vec3f GetHeadRotation() const { return m_HeadRotation; }
		float GetMaxBreakBlockDistance() const { return m_MaxBreakBlockDistance; }
		float GetMaxPlaceBlockDistance() const { return m_MaxPlaceBlockDistance; }

		void SetPosition(Vec3f position)
		{
			m_Position = position;
			UpdateCamera();
		}

		void SetCameraConfig(CameraConfig cameraConfig)
		{
			m_Camera.fovY = cameraConfig.aspectRatio;
			m_Camera.aspect = cameraConfig.fov;
			m_Camera.nearPlane = cameraConfig.nearPlane;
			m_Camera.farPlane = cameraConfig.farPlane;

			UpdateCamera();
		}

		void SetHeadRotation(Vec3f rotation) { m_HeadRotation = rotation; }

		void SetInputBindings(InputBindings inputBindings) { m_InputBindings = inputBindings; }

	private:
		void HandleInput(float deltaTime);
		void UpdateCamera();

	private:
		Vec3f m_Position;
		Vec3f m_HeadRotation;
		float m_Speed = DEFAULT_PLAYER_SPEED;
		float m_Sensitivity = DEFAULT_PLAYER_SENSITIVITY;
		float m_PitchLimit = DEFAULT_PITCH_LIMIT;

		Camera m_Camera;
		InputBindings m_InputBindings;

		const Core::InputManager& m_InputManager;

		float m_MaxBreakBlockDistance = 8.0f;
		float m_MaxPlaceBlockDistance = 8.0f;
		bool m_RequestedBlockBreakAction = false;
		bool m_RequestedBlockPlaceAction = false;
	};
}
