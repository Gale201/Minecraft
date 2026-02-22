#include "Player.h"

#include <algorithm>

namespace Minecraft
{
	Player::Player(const Core::InputManager& inputManager, InputBindings inputBindings, CameraConfig cameraConfig) :
		m_InputManager(inputManager),
		m_InputBindings(inputBindings),
		m_Camera()
	{
		SetCameraConfig(cameraConfig);
	}

	void Player::Update(float deltaTime)
	{
		HandleInput();

		if (m_CancelSprint && (m_PhysicsBody.isGrounded || m_Flying))
		{
			m_Sprinting = false;
			m_CancelSprint = false;
		}
		else if (m_StartSprint && (m_PhysicsBody.isGrounded || m_Flying))
		{
			m_Sprinting = true;
			m_StartSprint = false;
		}

		if (m_PhysicsBody.position.y < -100.0f)
			m_PhysicsBody.position.y = 60.0f;
	}

	bool Player::RequestedBlockBreakAction()
	{
		return m_RequestedBlockBreakAction;
	}

	bool Player::RequestedBlockPlaceAction()
	{
		return m_RequestedBlockPlaceAction;
	}

	Block Player::GetRequestedBlockToPlace()
	{
		return { BlockType::Glass };
	}

	void Player::HandleInput()
	{
		Vec3f velocity;
		m_PhysicsBody.velocity.x = 0.0f;
		m_PhysicsBody.velocity.z = 0.0f;
		if (m_Flying) m_PhysicsBody.velocity.y = 0.0f;

		// Movement
		Vec3f upVector{ 0.0f, 1.0f, 0.0f };
		Vec3f forwardVector = -upVector.Cross(m_Camera.GetRight()).normalized();

		if (m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::MoveForward)))
			velocity += forwardVector;

		if (m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::MoveBackward)))
			velocity -= forwardVector;

		if (m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::MoveRight)))
			velocity += m_Camera.GetRight();

		if (m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::MoveLeft)))
			velocity -= m_Camera.GetRight();

		if (m_Flying && m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::MoveDown)))
			velocity -= upVector;

		if (m_FlyingAllowed && m_InputManager.IsKeyJustDoublePressed(m_InputBindings.GetKeyForAction(InputAction::Jump)))
		{
			m_Flying = !m_Flying;
			m_PhysicsBody.applyGravity = !m_PhysicsBody.applyGravity;
		}
		else if (!m_Flying && m_PhysicsBody.isGrounded && m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::Jump)))
			m_PhysicsBody.velocity += m_JumpVelocity;
		else if (m_Flying && m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::MoveUp)))
			velocity += upVector;

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
	
		if (m_Sprinting)
			velocity = velocity.normalized() * m_SprintSpeed;
		else
			velocity = velocity.normalized() * m_Speed;
		m_PhysicsBody.velocity += velocity;

		// Look
		m_HeadRotation.y += m_InputManager.GetMouseMotionX() * m_Sensitivity;
		m_HeadRotation.x += m_InputManager.GetMouseMotionY() * m_Sensitivity;
		m_HeadRotation.x = std::clamp(m_HeadRotation.x, -m_PitchLimit, m_PitchLimit);

		// Placing/Destroying blocks
		m_RequestedBlockBreakAction = false;
		m_RequestedBlockPlaceAction = false;

		if (m_InputManager.IsMouseButtonJustReleased(m_InputBindings.GetMouseButtonForAction(InputAction::BreakBlock)))
			m_RequestedBlockBreakAction = true;

		if (m_InputManager.IsMouseButtonJustReleased(m_InputBindings.GetMouseButtonForAction(InputAction::PlaceBlock)))
			m_RequestedBlockPlaceAction = true;
	}

	void Player::SyncCamera()
	{
		m_Camera.position = m_PhysicsBody.position + m_RelativeCameraPos;
		m_Camera.position = Vec3f(0, 46, 0);
		m_Camera.pitch = m_HeadRotation.x;
		m_Camera.yaw = m_HeadRotation.y;
		m_Camera.roll = m_HeadRotation.z;
	}
}