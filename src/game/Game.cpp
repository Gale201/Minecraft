#include "Game.h"
#include "../core/Log.h"
#include "../timer/Timer.h"

namespace Minecraft
{
	Game::Game(int width, int height, std::string& name, Core::ThreadSystem& threadSystem, Core::InputManager& inputManager) :
		m_Width(width),
		m_Height(height),
		m_ThreadSystem(threadSystem), 
		m_InputManager(inputManager),
		m_Player(inputManager)
	{
		m_ThreadSystem.Initialize();

		m_Renderer.Initialize(m_Width, m_Height);
		m_Renderer.CreateTiles();

		m_RenderList.reserve(10000);

		bool successLoading = m_TextureLoader.LoadAllTextures();
		if (!successLoading)
		{
			LOG_ERROR("Error loading textures. Exiting...");
			exit(1);
		}

		m_World = m_WorldLoader.CreateWorld(name, 0);
		m_BlockInteractionSystem = std::make_unique<BlockInteractionSystem>(*m_World);
	}

	void Game::Setup()
	{
		m_Renderer.SetDirectionalLight({ -0.4f, -0.6f, -1.0f });

		m_World->SetTextureAtlas(m_TextureLoader.Get("atlas"));

		// Player setup
		InputBindings inputBindings;
		inputBindings.AddKeyBinding(InputAction::MoveForward, Key::W);
		inputBindings.AddKeyBinding(InputAction::MoveLeft, Key::A);
		inputBindings.AddKeyBinding(InputAction::MoveBackward, Key::S);
		inputBindings.AddKeyBinding(InputAction::MoveRight, Key::D);
		inputBindings.AddKeyBinding(InputAction::MoveUp, Key::Space);
		inputBindings.AddKeyBinding(InputAction::MoveDown, Key::Shift);
		inputBindings.AddMouseButtonBinding(InputAction::BreakBlock, MouseButton::Left);
		inputBindings.AddMouseButtonBinding(InputAction::PlaceBlock, MouseButton::Right);
		m_Player.SetInputBindings(inputBindings);

		CameraConfig cameraConfig{ 1.0f, static_cast<float>(m_Width) / static_cast<float>(m_Height), 0.01f, 100.0f };
		m_Player.SetCameraConfig(cameraConfig);
		m_Player.SetPosition(Vec3f(0.0f, 50.0f, 0.0f)); 

		m_Crosshair = m_TextureLoader.Get("crosshair");
	}

	void Game::Update(float deltaTime)
	{
		m_Player.Update(deltaTime);

		// Updating world
		m_World->LoadAndUnloadChunks(m_Player.GetCamera().position);
		m_World->Update(m_ThreadSystem);

		if (m_Player.RequestedBlockBreakAction())
			m_BlockInteractionSystem->BreakBlock(m_Player);

		if (m_Player.RequestedBlockPlaceAction())
			m_BlockInteractionSystem->PlaceBlock(m_Player, m_Player.GetRequestedBlockToPlace());

		m_World->BuildRenderList(m_Player.GetCamera(), m_RenderList);
	}

	void Game::Render()
	{
		m_Renderer.Clear(SKY_COLOR);

		{
			Core::Timer timer("Render");
			m_Renderer.Render(m_RenderList, m_Player.GetCamera(), m_ThreadSystem);
		}

		m_BlockInteractionSystem->Render(m_Player, m_Renderer);

		m_Renderer.DrawTexture(
			m_Width / 2 - m_Crosshair.GetWidth() / 2,
			m_Height / 2 - m_Crosshair.GetHeight() / 2,
			m_Crosshair, true
		);

		m_RenderList.clear();
	}
}