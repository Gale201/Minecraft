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
		m_PhysicsBody.size = m_Size;
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

	void Player::HandleInput()
	{
		Vec3f force;

		// Movement
		Vec3f upVector{ 0.0f, 1.0f, 0.0f };
		Vec3f forwardVector = -upVector.Cross(m_Camera.GetRight()).normalized();

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
		else if (m_PhysicsBody.inWater && m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::MoveDown)))
			m_PhysicsBody.AddForce({ 0.0f, -40.0f, 0.0f });

		if (m_FlyingAllowed && m_InputManager.IsKeyJustDoublePressed(m_InputBindings.GetKeyForAction(InputAction::Jump)))
		{
			m_Flying = !m_Flying;
			m_PhysicsBody.applyGravity = !m_PhysicsBody.applyGravity;
		}
		else if (!m_PhysicsBody.inWater && !m_Flying && m_PhysicsBody.isGrounded && m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::Jump)))
			m_PhysicsBody.AddForce(m_JumpVelocity);
		else if (m_PhysicsBody.inWater && !m_Flying && m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::MoveUp)))
			m_PhysicsBody.AddForce({ 0.0f, 40.0f, 0.0f });
		else if (m_Flying && m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::MoveUp)))
			force += upVector;

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
	
		if (m_Sprinting && !m_PhysicsBody.inWater)
			force = force.normalized() * m_SprintSpeed;
		else
			force = force.normalized() * m_Speed;

		if (m_Flying)
			m_PhysicsBody.flyingFriction = 10.0f;
		else
			m_PhysicsBody.flyingFriction = 0.0f;

		float forceMultiplier = m_PhysicsBody.isGrounded ? 10.0f : 2.f;
		if (m_Flying)
			force = { force.x, force.y * 12.0f, force.z };
		if (m_PhysicsBody.inWater) forceMultiplier = 3.0f;
		m_PhysicsBody.AddForce(force * forceMultiplier);

		// Look
		m_HeadRotation.y += m_InputManager.GetMouseMotionX() * m_Sensitivity;
		if (m_HeadRotation.y < -2.0f * PI) m_HeadRotation.y += 2.0f * PI;
		if (m_HeadRotation.y > 2.0f * PI) m_HeadRotation.y -= 2.0f * PI;
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

}