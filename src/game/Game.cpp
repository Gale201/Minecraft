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
		m_Player(inputManager),
		m_GameSaveManager("saves/worlds/", name, m_ItemRegistry),
		m_PhysicsSystem(threadSystem)
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

		srand(time(0));
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
			Vec3f playerPos(0, 70, 0);
			Chunk& chunk = m_World->GetChunk({ 0, 0, 0 });
			for (int y = 0; y < CHUNK_HEIGHT; y++)
			{
				if (chunk.blocks[0][y][0].type == BlockType::Grass) playerPos.y = y + 1;
			}
			m_Player.SetPosition(playerPos);
		}

		m_BlockInteractionSystem.Initialize(*m_World);
		m_PhysicsSystem.Initialize(*m_World);
		m_ItemRegistry.Initialize();

		// m_Player.SetGameMode(GameModeType::Survival);
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
		m_Player.SetInputBindings(inputBindings);

		CameraConfig cameraConfig{ 1.0f, static_cast<float>(m_Width) / static_cast<float>(m_Height), 0.01f, 1000.0f };
		m_Player.SetCameraConfig(cameraConfig);

		m_WaterScreenTexture = Texture(m_Width, m_Height);
		for (int i = 0; i < m_Width; i++)
			for (int j = 0; j < m_Height; j++)
				m_WaterScreenTexture.SetPixel(i, j, 0x334444FF);
		m_WaterScreenTexture.SetDefaultColor(0x334444FF);

		m_Player.SetInventoryTexture(m_TextureLoader.Get("inventory"));

		m_PhysicsSystem.AddBody(m_Player.GetBody());
	}

	void Game::Update(float deltaTime)
	{
		Core::Timer timer("Update");

		// Updating world
		m_World->LoadAndUnloadChunks(m_Player.GetCamera().position, m_ThreadSystem);
		m_World->Update(m_ThreadSystem);

		if (m_Player.RequestedBlockBreakAction())
			m_BlockInteractionSystem.BreakBlock(m_Player);

		if (m_Player.RequestedBlockPlaceAction())
			m_BlockInteractionSystem.PlaceBlock(m_Player, m_Player.GetRequestedBlockToPlace());

		while (!m_World->GetPhysicsAddQueue().empty())
		{
			PhysicsBody* body = m_World->GetPhysicsAddQueue().front();
			m_PhysicsSystem.AddBody(*body);
			m_World->GetPhysicsAddQueue().pop();
		}

		m_Player.Update(deltaTime);

		if (m_Player.IsInventoryOpen())
			m_MouseMode = Core::MouseMode::Normal;
		else
			m_MouseMode = Core::MouseMode::Captured;

		// Physics update
		m_PhysicsAccumulator += deltaTime;

		while (m_PhysicsAccumulator >= m_PhysicsFixedDelta)
		{
			m_Player.UpdatePhysics(deltaTime);

			m_PhysicsSystem.Update(m_PhysicsFixedDelta);
			m_PhysicsAccumulator -= m_PhysicsFixedDelta;
		}

		m_Player.SyncCamera();

		m_World->BuildRenderList(m_Player.GetCamera(), m_RenderList);
	}

	void Game::Render()
	{
		Core::Timer timer("Render");	

		m_Renderer.Clear(SKY_COLOR);

		m_Renderer.Render(m_RenderList, m_Player.GetCamera(), m_ThreadSystem);

		Mesh waterMesh;
		if (m_Player.IsInWater() && m_Player.GetHeadPosition().y + 0.1f < m_World->GetSeaLevel())
		{
			//m_Renderer.DrawTexture(0, 0, m_WaterScreenTexture, true);
			//RenderWaterMesh(waterMesh);
		}

		m_BlockInteractionSystem.Render(m_Player, m_Renderer);

		m_Renderer.DrawTexture(
			m_Width / 2 - m_TextureLoader.Get("crosshair").GetWidth() / 2,
			m_Height / 2 - m_TextureLoader.Get("crosshair").GetHeight() / 2,
			m_TextureLoader.Get("crosshair"), true
		);

		m_Player.Render(m_Renderer);

		Transform t1;
		t1.SetPosition(m_Player.GetPosition());
		t1.SetScale(m_Player.GetBody().size);
		//m_Renderer.DrawCubeWireframe(t1, 0xAAFF0000, m_Player.GetCamera(), 4, true, false);
		
		try
		{
			auto ls = m_World->GetItems().at(m_World->GetChunkPosition(Vec3i::floor(m_Player.GetPosition())));

			for (const Item& item : ls)
			{
				Transform t;
				t.SetPosition(item.body.position);
				t.SetScale(item.body.size);
				//m_Renderer.DrawCubeWireframe(t, 0xAAFF0000, m_Player.GetCamera(), 4, true, false);
			}
		}
		catch (std::exception e) { printf("%s\n", e.what()); }

		m_RenderList.clear();

		// UI Rendering
		m_Player.RenderUI(m_Renderer);
	}

	void Game::Resize(int width, int height)
	{
		m_Width = width;
		m_Height = height;
		m_Renderer.Initialize(m_Width, m_Height);
		m_Renderer.CreateTiles();
		m_Player.UpdateCameraAspectRatio(static_cast<float>(m_Width) / static_cast<float>(m_Height));
	}

	void Game::RenderWaterMesh(Mesh& mesh)
	{
		Vec3f headPos = m_Player.GetHeadPosition();
		Vec3f forward = {
			cos(m_Player.GetCamera().pitch) * sin(m_Player.GetCamera().yaw),
			sin(m_Player.GetCamera().pitch),
			cos(m_Player.GetCamera().pitch) * cos(m_Player.GetCamera().yaw)
		};
		forward = -forward.normalized();

		Vec3f worldUp = { 0, 1, 0 };

		Vec3f right = worldUp.Cross(forward).normalized() * 4.f;
		Vec3f up = forward.Cross(right).normalized();

		Vec3f vertices[4] = {
			headPos + forward * 0.1f + right + up,
			headPos + forward * 0.1f + right - up,
			headPos + forward * 0.1f - right - up,
			headPos + forward * 0.1f - right + up
		};
		vertices[0].y = std::min(vertices[0].y, (float)m_World->GetSeaLevel() - 1.0f + GetBlockBoundingBox(BlockType::Water).size.y);
		vertices[3].y = std::min(vertices[3].y, (float)m_World->GetSeaLevel() - 1.0f + GetBlockBoundingBox(BlockType::Water).size.y);

		Vec2f uvs[4] = { {1,1}, {1,0}, {0,0}, {0,1} };
		for (int i = 0; i < 4; i++)
		{
			Vertex v;
			v.position = vertices[i];
			v.u = uvs[i].x;
			v.v = uvs[i].y;
			v.tileInfo.isTransparent = true;

			v.tileInfo.u0 = 0.0f;
			v.tileInfo.v0 = 0.0f;
			v.tileInfo.uScale = 1.0f;
			v.tileInfo.vScale = 1.0f;
			v.tileInfo.atlasWidth = 256;
			v.tileInfo.atlasHeight = 255;

			mesh.vertices.push_back(v);
		}
		mesh.indices.push_back(0);
		mesh.indices.push_back(1);
		mesh.indices.push_back(2);
		mesh.indices.push_back(0);
		mesh.indices.push_back(2);
		mesh.indices.push_back(3);

		mesh.shade = false;

		m_RenderList.emplace_back(&mesh, Transform(), m_WaterScreenTexture);
	}

	void Game::Save()
	{
		m_GameSaveManager.SaveWorld(*m_World);
		m_GameSaveManager.SavePlayer(m_Player);
	}
}