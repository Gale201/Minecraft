#include "Minecraft.h"

#include "Clock.h"
#include "Log.h"
#include <string>

Minecraft::Minecraft()
{
	if (!window.Create(WIDTH, HEIGHT, TITLE)) return;

	renderer.Initialize(WIDTH, HEIGHT);
}

void Minecraft::Run()
{
	std::vector<Event> events;

	Clock clock(TARGET_FPS);

	bool running = true;
	while (running)
	{
		clock.Tick();
		LOG_INFO(("FPS: " + std::to_string(clock.GetFps())).c_str());

		// Poll events
		events.clear();
		window.PollEvents(running, events);

		// Process input events
		inputManager.ProcessEvents(events);

		// Update
		Update(clock.GetDeltaTime());

		// Render
		Render();

		// Render the frame buffer to the window
		window.Render(renderer.GetFrameBuffer());
	}

	window.Destroy();
}

void Minecraft::Update(float deltaTime)
{
	
}

void Minecraft::Render()
{
	renderer.Clear(0xFF000000);
}