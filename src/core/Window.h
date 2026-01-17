#pragma once

#include <cstdint>
#include <vector>

#include "Event.h"

/*
 Window class handles the creation of a window and rendering a frame buffer to it,
 as well as polling the input events.
*/
class Window
{
public:
	Window() = default;
	~Window() {}

	bool Create(int width, int height, const char* title);
	void Destroy();

	void PollEvents(bool& running, std::vector<Event>& outEvents);
	void Render(const uint32_t* frameBuffer);

	int GetWidth() const { return m_Width; }
	int GetHeight() const { return m_Height; }

private:
	int m_Width = 0;
	int m_Height = 0;

	void* m_WindowHandle = nullptr;
	void* m_Renderer = nullptr;
	void* m_Texture = nullptr;
};
