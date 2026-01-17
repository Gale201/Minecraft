#pragma once

#include <vector>

#include "Window.h"
#include "../render/Renderer.h"
#include "InputManager.h"

#define TITLE "Minecraft"
#define WIDTH 720
#define HEIGHT 480

#define TARGET_FPS 60

class Minecraft
{
public:
	Minecraft();

	void Run();
	void Update(float deltaTime);
	void Render();
	
private:
	Window window;
	Renderer renderer;
	InputManager inputManager;
};