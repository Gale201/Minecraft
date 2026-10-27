#include "Game.h"

#include <cstdlib>
#include <ctime>

#include "../core/Log.h"
#include "../timer/Timer.h"

namespace Minecraft
{
	Game::Game(int width, int height, const std::string& name, Core::ThreadSystem& threadSystem, Core::InputManager& inputManager) :
		m_Width(width),
		m_Height(height),
		m_ThreadSystem(threadSystem), 
		m_InputManager(inputManager),
		m_GameSaveManager("saves/worlds/", name, m_ItemRegistry),
		m_PhysicsSystem(threadSystem),
		m_Player(inputManager, m_PhysicsSystem)
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
		m_FontLoader.LoadFont(m_TextureLoader.Get("font"));

		srand(time(0));
	}

	void Game::Load(unsigned int seed)
	{
		m_World = m_GameSaveManager.LoadWorld();
		const bool worldExists = m_World != nullptr;
		if (worldExists)
		{
			m_GameSaveManager.LoadWorldItems(*m_World, m_PhysicsSystem);
			PlayerSaveData playerData = m_GameSaveManager.LoadPlayerData();
			m_Player.SetInitialPosition(playerData.position);
			m_Player.SetHeadRotation(playerData.headRotation);
			m_Player.GetInventory().SetSelectedHotbarSlotIndex(playerData.selectedHotbarSlotIndex);
			m_Player.GetInventory().GetSlots() = playerData.inventorySlots;
		}
		else
		{
			m_World = m_GameSaveManager.CreateWorld(seed);
			Vec3f playerPos(0, 70, 0);
			Chunk& chunk = m_World->GetChunk({ 0, 0, 0 });
			for (int y = 0; y < CHUNK_HEIGHT; y++)
			{
				if (chunk.blocks[0][y][0].type == BlockType::Grass) playerPos.y = y + 1;
			}
			m_Player.SetInitialPosition(playerPos);
		}

		m_BlockInteractionSystem.Initialize(*m_World);
		m_PhysicsSystem.Initialize(*m_World);
		m_ItemRegistry.Initialize();

		while (!m_World->GetPhysicsAddQueue().empty())
		{
			auto& [index, body] = m_World->GetPhysicsAddQueue().front();
			PhysicsBody* bodyPtr = m_PhysicsSystem.AddBody(body);
			m_World->GetItems()[index].body = bodyPtr;
			m_World->GetPhysicsAddQueue().pop();
		}

		bool successLoadingModels = m_EntityModelManager.LoadAllModels();
		if (!successLoadingModels)
		{
			LOG_ERROR("Error loading models. Exiting...");
			exit(1);
		}
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
		inputBindings.AddKeyBinding(InputAction::Sneak, Key::Shift);
		inputBindings.AddMouseButtonBinding(InputAction::BreakBlock, MouseButton::Left);
		inputBindings.AddMouseButtonBinding(InputAction::PlaceBlock, MouseButton::Right);
		inputBindings.AddKeyBinding(InputAction::OpenInventory, Key::E);
		inputBindings.AddKeyBinding(InputAction::DropItem, Key::Q);
		inputBindings.AddMouseButtonBinding(InputAction::PickBlock, MouseButton::Middle);
		inputBindings.AddKeyBinding(InputAction::OpenDebugTools, Key::F3);
		m_Player.SetInputBindings(inputBindings);

		m_Player.SetPhysicsBodyPtr(m_PhysicsSystem.AddBody(
			PhysicsBody(m_Player.GetInitialPosition(), m_Player.GetInitialSize())
		));

		CameraConfig cameraConfig{ 1.0f, static_cast<float>(m_Width) / static_cast<float>(m_Height), 0.01f, 1000.0f };
		m_Player.SetCameraConfig(cameraConfig);

		m_Player.SetTextures(
			m_TextureLoader.Get("crosshair"),
			m_TextureLoader.Get("steve-skin")
		);
		m_Player.GetInventory().SetTextures(
			m_TextureLoader.Get("inventory"),
			m_TextureLoader.Get("hotbar"),
			m_TextureLoader.Get("selected-slot"),
			m_TextureLoader.Get("icons"),
			m_TextureLoader.Get("steve-skin")
		);

		EntityModel playerModel = m_EntityModelManager.CreateEntityModel("player");
		playerModel.SetScale(1.f / 20.f);
		m_Player.SetModel(playerModel);

		m_Player.SetGameMode(GameModeType::Creative);
	}

	void Game::Update(float deltaTime)
	{
		Core::Timer timer(m_UpdateTime);

		// Updating world
		m_World->LoadAndUnloadChunks(m_Player.GetCamera().position, m_ThreadSystem);

		if (m_Player.RequestedBlockBreakAction())
			m_BlockInteractionSystem.BreakBlock(m_Player);

		if (m_Player.RequestedBlockPlaceAction())
			m_BlockInteractionSystem.PlaceBlock(m_Player, m_Player.GetRequestedBlockToPlace());

		m_World->Update(m_ThreadSystem, deltaTime);

		while (!m_World->GetPhysicsAddQueue().empty())
		{
			auto& [index, body] = m_World->GetPhysicsAddQueue().front();
			PhysicsBody* bodyPtr = m_PhysicsSystem.AddBody(body);
			m_World->GetItems()[index].body = bodyPtr;
			m_World->GetPhysicsAddQueue().pop();
		}

		m_Player.Update(deltaTime, *m_World, m_DebugOverlay);
		m_Player.SyncCamera();

		if (m_Player.IsInventoryOpen())
			m_MouseMode = Core::MouseMode::Normal;
		else
			m_MouseMode = Core::MouseMode::Captured;

		// Physics update
		m_PhysicsAccumulator += deltaTime;

		while (m_PhysicsAccumulator >= m_PhysicsFixedDelta)
		{
			m_Player.UpdatePhysics(m_PhysicsFixedDelta);

			m_PhysicsSystem.Update(m_PhysicsFixedDelta);
			m_PhysicsAccumulator -= m_PhysicsFixedDelta;
		}

		m_Player.HandleItemPickup(*m_World, m_PhysicsSystem, m_PhysicsFixedDelta);
		m_Player.SyncCamera(m_PhysicsAccumulator / m_PhysicsFixedDelta);

		m_World->BuildRenderList(m_Player.GetCamera(), m_RenderList);

		m_DebugOverlay.Update(m_Fps, m_FrameTime, m_UpdateTime, m_RenderTime, m_World->GetStats(), m_Renderer.GetStats());
	}

	void Game::Render()
	{
		Core::Timer timer(m_RenderTime);	

		m_Renderer.Clear(SKY_COLOR);

		m_Renderer.Render(m_RenderList, m_Player.GetCamera(), m_ThreadSystem);

		m_BlockInteractionSystem.Render(m_Player, m_Renderer);
		
		m_Player.Render(m_Renderer, m_ItemRegistry, m_World->GetSeaLevel());


		// Temp: item debug rendering
		for (Item& item : m_World->GetItems())
		{
			if (item.stack.size == 0 || item.body == nullptr) continue;

			Transform t;
			t.SetPosition(item.body->position);
			t.SetScale(item.body->size);
			//m_Renderer.DrawCubeWireframe(t, 0xAAFF0000, m_Player.GetCamera(), 4, true, false);
		}

		m_RenderList.clear();

		// UI Rendering
		m_Player.RenderUI(m_Renderer, m_ItemRegistry, m_FontLoader);

		m_DebugOverlay.Render(m_Renderer, m_FontLoader);
	}

	void Game::Resize(int width, int height)
	{
		m_Width = width;
		m_Height = height;
		m_Renderer.Initialize(m_Width, m_Height);
		m_Renderer.CreateTiles();
		m_Player.SetCameraAspectRatio(static_cast<float>(m_Width) / static_cast<float>(m_Height));
	}

	void Game::Save()
	{
		LOG_INFO("Saving world...");
		m_GameSaveManager.SaveWorld(*m_World);
		m_GameSaveManager.SavePlayer(m_Player);
		m_GameSaveManager.SaveWorldItems(*m_World);
	}
}