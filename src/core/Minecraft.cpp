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
}

void Minecraft::Run()
{

	m_Renderer.SetDirectionalLight({ -0.3f, -0.6f, -1 });

	InputBindings inputBindings;
	inputBindings.AddKeyBinding(InputAction::MoveForward, Key::W);
	inputBindings.AddKeyBinding(InputAction::MoveLeft, Key::A);
	inputBindings.AddKeyBinding(InputAction::MoveBackward, Key::S);
	inputBindings.AddKeyBinding(InputAction::MoveRight, Key::D);
	inputBindings.AddKeyBinding(InputAction::MoveUp, Key::Space);
	inputBindings.AddKeyBinding(InputAction::MoveDown, Key::Shift);

	CameraController cameraController{ m_Camera, m_InputManager, inputBindings };

	std::optional<Texture> textureAtlas = m_TextureLoader.LoadTexture("texture-atlas.png");
	if (textureAtlas.has_value())
		m_World.SetTextureAtlas(textureAtlas.value());

	std::optional<Texture> crosshair = m_TextureLoader.LoadTexture("crosshair.png");
	for (int i = 0; i < crosshair.value().GetWidth() * crosshair.value().GetHeight(); i++)
	{
		uint32_t color = crosshair.value().GetPixels()[i];
		uint8_t a = (color >> 24) & 0xFF;
		color &= 0x00FFFFFF;

		float alpha = a / 255.0f;
		alpha = std::fmaxf(alpha - 0.4f, 0.0f);

		uint8_t newAlpha = (uint8_t)(alpha * 255.0f);
		color |= newAlpha << 24;

		crosshair.value().SetPixelByIndex(i, color);
	}

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
		m_Renderer.DrawTexture(WIDTH / 2 - crosshair.value().GetWidth() / 2, HEIGHT / 2 - crosshair.value().GetHeight() / 2, crosshair.value(), true);
		
		// Render the frame buffer to the window
		m_Window.Render(m_Renderer.GetFrameBuffer());
	}

	m_ThreadSystem.Shutdown();
	m_Window.Destroy();
}

void Minecraft::Update(float deltaTime)
{
	m_World.LoadAndUnloadChunks(m_Camera.position);
	{
		Timer timer("Building Chunk Meshes");
		m_World.Update(m_ThreadSystem);
	}

	WorldRaycastResult raycast;
	if (m_World.Raycast(m_Camera.position, m_Camera.GetForward(), 8.0f, raycast))
	{
		if (m_InputManager.IsMouseButtonJustReleased(MouseButton::Left))
			m_World.SetBlock(raycast.blockPosition, BlockType::Air);
		else if (m_InputManager.IsMouseButtonJustReleased(MouseButton::Right))
		{
			Vec3i normal = Vec3i(
				static_cast<int>(raycast.normal.x),
				static_cast<int>(raycast.normal.y),
				static_cast<int>(raycast.normal.z)
			);
			m_World.SetBlock(raycast.blockPosition + normal, BlockType::Glass);
		}
	}

	{
		Timer timer("Build Render List");
		m_World.BuildRenderList(m_Camera, m_RenderList);
	}
}

void Minecraft::Render()
{
	m_Renderer.Clear(SKY_COLOR);

	{
		Timer timer("Render");
		m_Renderer.Render(m_RenderList, m_Camera, m_ThreadSystem);
	}

	WorldRaycastResult raycast;
	if (m_World.Raycast(m_Camera.position, m_Camera.GetForward(), 8.0f, raycast))
	{
		Transform t;

		t.SetScale(Vec3f(1.0001f, 1.0001f, 1.0001f));
		t.SetPosition(raycast.blockPosition.toVec3f() + Vec3f(0.5f, 0.5f, 0.5f));
		m_Renderer.DrawCubeWireframe(t, 0x88222222, m_Camera, 2, true);
	}

	m_RenderList.clear();
}