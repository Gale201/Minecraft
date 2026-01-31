#pragma once

#include <vector>

#include "Window.h"
#include "../render/Renderer.h"
#include "InputManager.h"
#include "../loaders/ModelLoader.h"
#include "../loaders/TextureLoader.h"
#include "../world/World.h"
#include "../render/Camera.h"

#define TITLE "Minecraft"
#define WIDTH 1280
#define HEIGHT 720

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
	World m_World;
	Renderer m_Renderer;
	InputManager m_InputManager;
	
	ModelLoader m_ModelLoader{ "res/objs/" };
	TextureLoader m_TextureLoader{ "res/textures/" };

	Camera m_Camera{ {0,20,40}, 0.0f, 0.0f, 0.0f, 1.0f, (float)WIDTH / (float)HEIGHT, 0.1f, 100 };
	
	Texture m_TextureAtlas;
};