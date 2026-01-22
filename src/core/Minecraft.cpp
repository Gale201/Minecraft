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
	std::optional<Model> model = m_ModelLoader.LoadModelFromObj("cube.obj");
	if (!model.has_value())
	{
		LOG_INFO("Failed to load cube.obj!");
		return;
	}

	std::optional<Texture> texture = m_TextureLoader.LoadTexture("grass.png");
	if (!texture.has_value())
	{
		LOG_INFO("Failed to load textures!");
		return;
	}

	std::vector<RenderObject> rObjs;
	for (int i = 0; i < 16 * 16; i++)
	{
		Transform transform;
		transform.SetPosition({ (float) (i % 16) * 2, 0.0f, (float) (i / 16) * 2 });
		rObjs.emplace_back(model.value(), transform, texture.value());
	}
	m_Renderer.SetDirectionalLight({ 0.3, 1, 0.6 });

	Camera camera{ {0,0,50}, 0.0f, 0.0f, 0.0f, 1.0f, (float)WIDTH / (float)HEIGHT, 0.1f, 100 };

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

		for (RenderObject& obj : rObjs)
			m_Renderer.DrawRenderObject(obj, camera);

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