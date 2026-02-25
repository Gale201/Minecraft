#pragma once

#include <vector>

#include "Window.h"
#include "InputManager.h"
#include "../thread/ThreadSystem.h"
#include "../game/Game.h"

#define TITLE "Minecraft"
#define WIDTH 1920
#define HEIGHT 1080

#define TARGET_FPS 60

namespace Core
{
	class Application
	{
	public:
		Application();

		void Run();
		void Update(float deltaTime);
		void Render();

	private:
		bool m_Running = false;

		Window m_Window;
		ThreadSystem m_ThreadSystem;
		InputManager m_InputManager;

		std::unique_ptr<Minecraft::Game> m_Game;
	};
}