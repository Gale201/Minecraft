#pragma once
#include "../render/Renderer.h"
#include "../loaders/TextureLoader.h"
#include "../player/Player.h"
#include "../world/BlockInteractionSystem.h"
#include "../world/GameSaveManager.h"
#include "../physics/PhysicsSystem.h"
#include "../items/ItemRegistry.h"

#define SKY_COLOR 0xFF99BBFF

namespace Minecraft
{
	class Game
	{
	public:
		Game(int width, int height, const std::string& name, Core::ThreadSystem&, Core::InputManager&);

		void Load(unsigned int seed = 0);
		void Setup();

		void Update(float deltaTime);
		void Render();

		void Resize(int width, int height);

		void RenderWaterMesh(Mesh& mesh);

		void Save();

		uint32_t* GetFrameBuffer() { return m_Renderer.GetFrameBuffer(); }
		Core::MouseMode GetMouseMode() const { return m_MouseMode; }

	private:
		int m_Width;
		int m_Height;
		Core::ThreadSystem& m_ThreadSystem;
		Core::InputManager& m_InputManager;

		Renderer m_Renderer;
		std::vector<RenderCommand> m_RenderList;

		TextureLoader m_TextureLoader{ "res/textures/" };
		ItemRegistry m_ItemRegistry{ m_TextureLoader };

		GameSaveManager m_GameSaveManager;

		std::unique_ptr<World> m_World;
		BlockInteractionSystem m_BlockInteractionSystem;
		PhysicsSystem m_PhysicsSystem;
		Player m_Player;

		float m_PhysicsAccumulator = 0.0f;
		const float m_PhysicsFixedDelta = 1.0f / 20.0f;

		Texture m_WaterScreenTexture;

		Core::MouseMode m_MouseMode = Core::MouseMode::Captured;
	};
}