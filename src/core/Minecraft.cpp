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

	m_Renderer.SetDirectionalLight({ -0.3f, -0.6f, -1 });

	InputBindings inputBindings;
	inputBindings.AddKeyBinding(InputAction::MoveForward, Key::W);
	inputBindings.AddKeyBinding(InputAction::MoveLeft, Key::A);
	inputBindings.AddKeyBinding(InputAction::MoveBackward, Key::S);
	inputBindings.AddKeyBinding(InputAction::MoveRight, Key::D);
	inputBindings.AddKeyBinding(InputAction::MoveUp, Key::SPACE);
	inputBindings.AddKeyBinding(InputAction::MoveDown, Key::SHIFT);

	CameraController cameraController{ m_Camera, m_InputManager, inputBindings };

	m_TextureAtlas = m_TextureLoader.LoadTexture("texture-atlas.png").value();

	auto genChunk = [&](int x, int z) {
		x *= 16;
		z *= 16;
		for (int j = 0; j < 4; j++)
			for (int i = 0; i < 16 * 16; i++)
			{
				m_World.SetBlock({ x + i / 16, j + 11, z + i % 16}, BlockType::Dirt);
			}

		for (int j = 0; j < 11; j++)
			for (int i = 0; i < 16 * 16; i++)
			{
				m_World.SetBlock({ x + i / 16, j, z + i % 16 }, BlockType::Stone);
			}
		for (int i = 0; i < 16 * 16; i++)
		{
			m_World.SetBlock({ x + i / 16, 15, z + i % 16 }, BlockType::Grass);
		}
		};

	for (int x = -3; x < 3; x++)
		for (int z = -3; z < 3; z++)
			genChunk(x, z);

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

		// Render the frame buffer to the window
		m_Window.Render(m_Renderer.GetFrameBuffer());
	}

	m_Window.Destroy();
}

void Minecraft::Update(float deltaTime)
{
	m_World.Update();
}

void Minecraft::Render()
{
	m_Renderer.Clear(0xFF000000);

	for (auto& [chunkPos, chunk] : m_World.GetChunks())
	{
		Transform t;
		t.SetPosition(m_World.GetChunkWorldPosition(chunk.position).toVec3f());
		m_Renderer.DrawMesh(chunk.mesh.mesh, t, m_TextureAtlas, m_Camera);
	}
}