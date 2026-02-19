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

	m_ThreadSystem.Initialize();

	m_Renderer.Initialize(WIDTH, HEIGHT);
	m_Renderer.CreateTiles();

	m_RenderList.reserve(10000);

	bool successLoading = m_TextureLoader.LoadAllTextures();
	if (!successLoading)
	{
		LOG_ERROR("Error loading textures. Exiting...");
		exit(1);
	}
}

void Minecraft::Run()
{
	m_Renderer.SetDirectionalLight({ -0.3f, -0.6f, -1.0f });

	InputBindings inputBindings;
	inputBindings.AddKeyBinding(InputAction::MoveForward, Key::W);
	inputBindings.AddKeyBinding(InputAction::MoveLeft, Key::A);
	inputBindings.AddKeyBinding(InputAction::MoveBackward, Key::S);
	inputBindings.AddKeyBinding(InputAction::MoveRight, Key::D);
	inputBindings.AddKeyBinding(InputAction::MoveUp, Key::Space);
	inputBindings.AddKeyBinding(InputAction::MoveDown, Key::Shift);
	inputBindings.AddMouseButtonBinding(InputAction::BreakBlock, MouseButton::Left);
	inputBindings.AddMouseButtonBinding(InputAction::PlaceBlock, MouseButton::Right);

	CameraConfig cameraConfig{ 1.0f, (float)WIDTH / (float)HEIGHT, 0.01f, 100.0f };
	m_Player = std::make_unique<Player>(m_InputManager, inputBindings, cameraConfig);
	m_Player->SetPosition({ 0, 50, 0 });

	m_World.SetTextureAtlas(m_TextureLoader.Get("atlas"));

	Texture& crosshair = m_TextureLoader.Get("crosshair");

	// Main loop

	std::vector<Event> events;

	Clock clock(TARGET_FPS);

	m_Running = true;
	while (m_Running)
	{
		std::cout << '\n';

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

		// Render
		Render();
		m_Renderer.DrawTexture(WIDTH / 2 - crosshair.GetWidth() / 2, HEIGHT / 2 - crosshair.GetHeight() / 2, crosshair, true);
		
		// Render the frame buffer to the window
		m_Window.Render(m_Renderer.GetFrameBuffer());
	}

	m_ThreadSystem.Shutdown();
	m_Window.Destroy();
}

void Minecraft::Update(float deltaTime)
{
	m_Player->Update(deltaTime);

	// Updating world
	m_World.LoadAndUnloadChunks(m_Player->GetCamera().position);
	m_World.Update(m_ThreadSystem);

	if (m_Player->RequestedBlockBreakAction())
	{
		m_BlockInteractionSystem.BreakBlock(*m_Player);
	}

	if (m_Player->RequestedBlockPlaceAction())
	{
		m_BlockInteractionSystem.PlaceBlock(*m_Player, m_Player->GetRequestedBlockToPlace());
	}

	{
		Timer timer("Build Render List");
		m_World.BuildRenderList(m_Player->GetCamera(), m_RenderList);
	}
}

void Minecraft::Render()
{
	m_Renderer.Clear(SKY_COLOR);

	{
		Timer timer("Render");
		m_Renderer.Render(m_RenderList, m_Player->GetCamera(), m_ThreadSystem);
	}

	m_BlockInteractionSystem.Render(*m_Player, m_Renderer);

	m_RenderList.clear();
}