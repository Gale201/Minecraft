#pragma once
#include "../render/Renderer.h"
#include "../world/WorldLoader.h"
#include "../loaders/TextureLoader.h"
#include "../player/Player.h"
#include "../world/BlockInteractionSystem.h"

#define SKY_COLOR 0xFF99BBFF

namespace Minecraft
{
	class Game
	{
	public:
		Game(int width, int height, std::string& name, Core::ThreadSystem&, Core::InputManager&);
		~Game() = default;

		void Setup();

		void Update(float deltaTime);
		void Render();

		uint32_t* GetFrameBuffer() { return m_Renderer.GetFrameBuffer(); }

	private:
		int m_Width;
		int m_Height;
		Core::ThreadSystem& m_ThreadSystem;
		Core::InputManager& m_InputManager;

		Minecraft::Renderer m_Renderer;
		std::vector<Minecraft::RenderCommand> m_RenderList;

		Minecraft::TextureLoader m_TextureLoader{ "res/textures/" };

		Minecraft::WorldLoader m_WorldLoader{ "saves/worlds/" };

		std::unique_ptr<World> m_World;
		std::unique_ptr<BlockInteractionSystem> m_BlockInteractionSystem;
		Player m_Player;

		Texture m_Crosshair;
	};
}