#include "Player.h"
#include <algorithm>

Player::Player(const InputManager& inputManager, InputBindings inputBindings, CameraConfig cameraConfig) :
	m_Position(0, 0, 0),
	m_HeadRotation(0, 0, 0),
	m_InputManager(inputManager),
	m_InputBindings(inputBindings),
	m_Camera()
{
	m_Camera.fovY = cameraConfig.aspectRatio;
	m_Camera.aspect = cameraConfig.fov;
	m_Camera.nearPlane = cameraConfig.nearPlane;
	m_Camera.farPlane = cameraConfig.farPlane;

	UpdateCamera();
}

void Player::Update(float deltaTime)
{
	HandleInput(deltaTime);
	UpdateCamera();
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

void Player::HandleInput(float deltaTime)
{
	// Movement
	Vec3f upVector{ 0.0f, 1.0f, 0.0f };
	Vec3f forwardVector = -upVector.Cross(m_Camera.GetRight()).normalized();

	if (m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::MoveForward)))
		m_Position += forwardVector * m_Speed * deltaTime;

	if (m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::MoveBackward)))
		m_Position -= forwardVector * m_Speed * deltaTime;

	if (m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::MoveRight)))
		m_Position += m_Camera.GetRight() * m_Speed * deltaTime;

	if (m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::MoveLeft)))
		m_Position -= m_Camera.GetRight() * m_Speed * deltaTime;

	if (m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::MoveUp)))
		m_Position += upVector * m_Speed * deltaTime;

	if (m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::MoveDown)))
		m_Position -= upVector * m_Speed * deltaTime;

	// Look
	m_HeadRotation.y += m_InputManager.GetMouseMotionX() * m_Sensitivity; // * deltaTime;
	m_HeadRotation.x += m_InputManager.GetMouseMotionY() * m_Sensitivity; // * deltaTime;
	m_HeadRotation.x = std::clamp(m_HeadRotation.x, -m_PitchLimit, m_PitchLimit);
	
	// Placing/Destroying blocks
	m_RequestedBlockBreakAction = false;
	m_RequestedBlockPlaceAction = false;

	if (m_InputManager.IsMouseButtonJustReleased(m_InputBindings.GetMouseButtonForAction(InputAction::BreakBlock)))
		m_RequestedBlockBreakAction = true;

	if (m_InputManager.IsMouseButtonJustReleased(m_InputBindings.GetMouseButtonForAction(InputAction::PlaceBlock)))
		m_RequestedBlockPlaceAction = true;
}

void Player::UpdateCamera()
{
	m_Camera.position = m_Position;
	m_Camera.pitch = m_HeadRotation.x;
	m_Camera.yaw = m_HeadRotation.y;
	m_Camera.roll = m_HeadRotation.z;
}