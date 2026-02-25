#include "Game.h"
#include "../core/Log.h"
#include "../timer/Timer.h"

namespace Minecraft
{
	Game::Game(int width, int height, const std::string& name, Core::ThreadSystem& threadSystem, Core::InputManager& inputManager) :
		m_Width(width),
		m_Height(height),
		m_ThreadSystem(threadSystem), 
		m_InputManager(inputManager),
		m_Player(inputManager),
		m_GameSaveManager("saves/worlds/", name)
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
	}

	void Game::Load(unsigned int seed)
	{
		if (m_GameSaveManager.WorldExists())
		{
			m_World = m_GameSaveManager.LoadWorld();
			PlayerSaveData playerData = m_GameSaveManager.LoadPlayerData();
			m_Player.SetPosition(playerData.position);
			m_Player.SetHeadRotation(playerData.headRotation);
		}
		else
		{
			m_World = m_GameSaveManager.CreateWorld(seed);
		}

		m_BlockInteractionSystem.Initialize(*m_World);
		m_PhysicsSystem.Initialize(*m_World);
	}

	void Game::Setup()
	{
		m_Renderer.SetDirectionalLight({ -0.4f, -0.6f, -1.0f });

		m_World->SetTextureAtlas(m_TextureLoader.Get("atlas-old"));

		// Player setup
		InputBindings inputBindings;
		inputBindings.AddKeyBinding(InputAction::MoveForward, Key::W);
		inputBindings.AddKeyBinding(InputAction::MoveLeft, Key::A);
		inputBindings.AddKeyBinding(InputAction::MoveBackward, Key::S);
		inputBindings.AddKeyBinding(InputAction::MoveRight, Key::D);
		inputBindings.AddKeyBinding(InputAction::MoveUp, Key::Space);
		inputBindings.AddKeyBinding(InputAction::MoveDown, Key::Shift);
		inputBindings.AddKeyBinding(InputAction::Jump, Key::Space);
		inputBindings.AddKeyBinding(InputAction::Sprint, Key::LeftCtrl);
		inputBindings.AddMouseButtonBinding(InputAction::BreakBlock, MouseButton::Left);
		inputBindings.AddMouseButtonBinding(InputAction::PlaceBlock, MouseButton::Right);
		m_Player.SetInputBindings(inputBindings);

		CameraConfig cameraConfig{ 1.0f, static_cast<float>(m_Width) / static_cast<float>(m_Height), 0.01f, 1000.0f };
		m_Player.SetCameraConfig(cameraConfig);

		m_Crosshair = m_TextureLoader.Get("crosshair");

		m_PhysicsSystem.AddBody(m_Player.GetBody());
	}

	void Game::Update(float deltaTime)
	{
		Core::Timer timer("Update");

		// Updating world
		m_World->LoadAndUnloadChunks(m_Player.GetCamera().position);
		m_World->Update(m_ThreadSystem);

		if (m_Player.RequestedBlockBreakAction())
			m_BlockInteractionSystem.BreakBlock(m_Player);

		if (m_Player.RequestedBlockPlaceAction())
			m_BlockInteractionSystem.PlaceBlock(m_Player, m_Player.GetRequestedBlockToPlace());

		m_World->BuildRenderList(m_Player.GetCamera(), m_RenderList);

		// Physics update
		m_PhysicsAccumulator += deltaTime;

		while (m_PhysicsAccumulator >= m_PhysicsFixedDelta)
		{
			m_Player.Update(deltaTime);

			m_PhysicsSystem.Update(m_PhysicsFixedDelta);
			m_PhysicsAccumulator -= m_PhysicsFixedDelta;
		}

		m_Player.SyncCamera();
	}

	void Game::Render()
	{
		Core::Timer timer("Render");

		m_Renderer.Clear(SKY_COLOR);

		m_Renderer.Render(m_RenderList, m_Player.GetCamera(), m_ThreadSystem);

		m_BlockInteractionSystem.Render(m_Player, m_Renderer);

		m_Renderer.DrawTexture(
			m_Width / 2 - m_Crosshair.GetWidth() / 2,
			m_Height / 2 - m_Crosshair.GetHeight() / 2,
			m_Crosshair, true
		);

		Transform t1;
		t1.SetPosition(m_Player.GetPosition());
		t1.SetScale(m_Player.GetBody().size);
		//m_Renderer.DrawCubeWireframe(t1, 0xAAFF0000, m_Player.GetCamera(), 4, true, false);

		m_RenderList.clear();
	}

	void Game::Save()
	{
		m_GameSaveManager.SaveWorld(*m_World);
		m_GameSaveManager.SavePlayer(m_Player);
	}
}