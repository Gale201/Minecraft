#include "Minecraft.h"

#include <string>

#include "Clock.h"
#include "Log.h"
#include "../render/RenderObject.h"
#include "../render/Camera.h"
#include "../render/CameraController.h"

Minecraft::Minecraft()
{
	if (!m_Window.Create(WIDTH, HEIGHT, TITLE)) return;

	m_Renderer.Initialize(WIDTH, HEIGHT);
}

void Minecraft::Run()
{
	Model model;
	m_ModelLoader.LoadModelFromObj("snowman.obj", model);
	RenderObject cube{ model, Transform(), 0xFFFF0000 };

	Camera camera{ {1,1,10}, 0.3, 0.4, 0, 0.5, (float)WIDTH / (float)HEIGHT, 0.1, 100 };

	InputBindings inputBindings;
	inputBindings.AddKeyBinding(InputAction::MoveForward, Key::W);
	inputBindings.AddKeyBinding(InputAction::MoveLeft, Key::A);
	inputBindings.AddKeyBinding(InputAction::MoveBackward, Key::S);
	inputBindings.AddKeyBinding(InputAction::MoveRight, Key::D);
	inputBindings.AddKeyBinding(InputAction::MoveUp, Key::SPACE);
	inputBindings.AddKeyBinding(InputAction::MoveDown, Key::SHIFT);

	CameraController cameraController{ camera, m_InputManager, inputBindings };

	// Main loop

	std::vector<Event> events;

	Clock clock(TARGET_FPS);

	m_Running = true;
	while (m_Running)
	{
		clock.Tick();
		LOG_INFO("FPS: %f", clock.GetFps());

		// Poll events
		events.clear();
		m_Window.PollEvents(m_Running, events);

		// Process input events
		m_InputManager.ProcessEvents(events);

		// Update
		Update(clock.GetDeltaTime());
		cameraController.Update(clock.GetDeltaTime());

		// Render
		Render();

		m_Renderer.DrawRenderObject(cube, camera);

		// Render the frame buffer to the window
		m_Window.Render(m_Renderer.GetFrameBuffer());
	}

	m_Window.Destroy();
}

void Minecraft::Update(float deltaTime)
{

}

void Minecraft::Render()
{
	m_Renderer.Clear(0xFF000000);
}