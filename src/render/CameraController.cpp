#include "CameraController.h"

#include <algorithm>

#include "../core/Log.h"

namespace Minecraft
{
	void CameraController::Update(float deltaTime)
	{
		Vec3f upVector{ 0.0f, 1.0f, 0.0f };
		Vec3f forwardVector = -upVector.Cross(m_Camera.GetRight()).normalized();

		if (m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::MoveForward)))
			m_Camera.position += forwardVector * m_CameraSpeed * deltaTime;

		if (m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::MoveBackward)))
			m_Camera.position -= forwardVector * m_CameraSpeed * deltaTime;

		if (m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::MoveRight)))
			m_Camera.position += m_Camera.GetRight() * m_CameraSpeed * deltaTime;

		if (m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::MoveLeft)))
			m_Camera.position -= m_Camera.GetRight() * m_CameraSpeed * deltaTime;

		if (m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::MoveUp)))
			m_Camera.position += upVector * m_CameraSpeed * deltaTime;

		if (m_InputManager.IsKeyDown(m_InputBindings.GetKeyForAction(InputAction::MoveDown)))
			m_Camera.position -= upVector * m_CameraSpeed * deltaTime;

		m_Camera.yaw += m_InputManager.GetMouseMotionX() * m_Sensitivity; // * deltaTime;
		m_Camera.pitch += m_InputManager.GetMouseMotionY() * m_Sensitivity; // * deltaTime;
		m_Camera.pitch = std::clamp(m_Camera.pitch, -m_PitchLimit, m_PitchLimit);
	}
}
