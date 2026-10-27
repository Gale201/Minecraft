#include "Application.h"

#include <string>

#include "Clock.h"
#include "Log.h"
#include "../timer/Timer.h"

namespace Core
{
	Application::Application()
	{
		if (!m_Window.Create(WIDTH, HEIGHT, TITLE)) exit(1);
	}

	void Application::Run()
	{
		std::string name = "Test";
		m_Game = std::make_unique<Minecraft::Game>(WIDTH, HEIGHT, name, m_ThreadSystem, m_InputManager);
		m_Game->Load(0x020209);
		m_Game->Setup();

		// Main loop
		std::vector<Event> events;

		Clock clock(TARGET_FPS);

		float frameTime = 0.0f;

		m_Running = true;
		while (m_Running)
		{
			clock.Tick();

			m_Game->SetFrameTiming(clock.GetFps(), frameTime);

			Timer frame(frameTime);

			// Poll events
			events.clear();
			m_Window.PollEvents(m_Running, events);

			// Process input events
			m_InputManager.ProcessEvents(events);

			// Update
			Update(clock.GetDeltaTime());

			// Render
			Render();
	
			// Render the frame buffer to the window
			m_Window.Render(m_Game->GetFrameBuffer());
		}

		m_Game->Save();

		m_ThreadSystem.Shutdown();
		m_Window.Destroy();
	}

	void Application::Update(float deltaTime)
	{
		if (m_InputManager.ResizeJustHappened())
		{
			m_Game->Resize(m_Window.GetWidth(), m_Window.GetHeight());
		}

		m_Game->Update(deltaTime);

		if (m_Window.isMouseRelative() != (m_Game->GetMouseMode() == MouseMode::Captured))
			m_Window.ResetMousePosition();
		m_Window.SetMouseMode(m_Game->GetMouseMode());
	}

	void Application::Render()
	{
		m_Game->Render();
	}
}
