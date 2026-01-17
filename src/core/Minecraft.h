#pragma once

#include <vector>

#include "Window.h"
#include "../render/Renderer.h"
#include "InputManager.h"
#include "../loaders/ModelLoader.h"

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
	bool m_Running = false;

	Window m_Window;
	Renderer m_Renderer;
	InputManager m_InputManager;
	
	ModelLoader m_ModelLoader{ "res/objs/" };
};