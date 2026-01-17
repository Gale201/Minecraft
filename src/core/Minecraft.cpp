#include "Minecraft.h"

#include "Clock.h"
#include "Log.h"
#include <string>

Minecraft::Minecraft()
{
	if (!window.Create(WIDTH, HEIGHT, TITLE)) return;

	renderer.Initialize(WIDTH, HEIGHT);

	Model model;
	modelLoader.LoadModelFromObj("cube.obj", model);

	for (int i = 0; i < model.meshes[0].vertices.size(); i++)
	{
		Vertex v = model.meshes[0].vertices[i];
		LOG_INFO("%f %f %f, %f %f, %f %f %f", v.position.x, v.position.y, v.position.z, v.u, v.v, v.normal.x, v.normal.y, v.normal.z);
	}
}

void Minecraft::Run()
{
	std::vector<Event> events;

	Clock clock(TARGET_FPS);

	bool running = true;
	while (running)
	{
		clock.Tick();
		LOG_INFO("FPS: %f", clock.GetFps());

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