#pragma once
#include "Camera.h"
#include "../core/InputManager.h"
#include "../game/input/InputBinding.h"

class CameraController
{
public:
	CameraController(Camera& camera, const InputManager& inputManager, InputBindings& inputBindings)
		: m_Camera(camera), m_InputManager(inputManager), m_InputBindings(inputBindings)
	{
	}

	~CameraController() = default;
	
	void Update(float deltaTime);

private:
	Camera& m_Camera;
	const InputManager& m_InputManager;
	InputBindings& m_InputBindings;

	float m_CameraSpeed = 10.0f;
	float m_Sensitivity = 0.001f;

	float m_PitchLimit = 1.55f;
};
