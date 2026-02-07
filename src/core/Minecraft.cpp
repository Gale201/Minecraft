#include "Minecraft.h"

#include <string>

#include "Clock.h"
#include "Log.h"
#include "../render/RenderObject.h"
#include "../render/Camera.h"
#include "../render/CameraController.h"
#include "../timer/Timer.h"
#include "../timer/AccumTimer.h"

Minecraft::Minecraft()
{
	if (!m_Window.Create(WIDTH, HEIGHT, TITLE)) return;

	m_Renderer.Initialize(WIDTH, HEIGHT);
	m_Renderer.CreateTiles();
	m_Renderer.InitThreads();

	m_RenderList.reserve(10000);
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

	std::optional<Texture> textureAtlas = m_TextureLoader.LoadTexture("texture-atlas.png");
	if (textureAtlas.has_value())
		m_World.SetTextureAtlas(textureAtlas.value());

	auto genChunk = [&](int x, int z) {
		x *= 16;
		z *= 16;
		for (int j = 0; j < 4; j++)
			for (int i = 0; i < 16 * 16; i++)
			{
				m_World.SetBlock({ x + i / 24, j + 11, z + i % 12}, BlockType::Dirt);
			}

		for (int j = 0; j < 11; j++)
			for (int i = 0; i < 16 * 16; i++)
			{
				m_World.SetBlock({ x + i / 32, j, z + i % 16 }, BlockType::Stone);
			}
		for (int i = 0; i < 16 * 16; i++)
		{
			m_World.SetBlock({ x + i / 20, 15, z + i % 4 }, BlockType::Grass);
		}
	};

	for (int x = -16; x < 16; x++)
		for (int z = -16; z < 16; z++)
			genChunk(x, z);

	// Main loop

	std::vector<Event> events;

	Clock clock(TARGET_FPS);

	m_Running = true;
	while (m_Running)
	{
		std::cout << std::endl;

		clock.Tick();
		LOG_INFO("FPS: %f", clock.GetFps());

		Timer frame("Frame");

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

	{
		Timer timer("Build Render List");
		m_World.BuildRenderList(m_Camera, m_RenderList);
	}

	printf("Render List Size: %d\n", static_cast<int>(m_RenderList.size()));
}


void Minecraft::Render()
{
	m_Renderer.Clear(0xFF000000);

	{
		Timer timer("Render");
		m_Renderer.Render(m_RenderList, m_Camera);
	}

	m_RenderList.clear();
}