#include "Window.h"

#include <SDL.h>

#include "Log.h"

bool Window::Create(int width, int height, const char* title)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		LOG_ERROR("SDL_Init failed: %s", SDL_GetError());
		return false;
	}

	m_WindowHandle = SDL_CreateWindow(
		title,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		width, height,
		0
	);

	if (!m_WindowHandle)
	{
		LOG_ERROR("SDL_CreateWindow failed!");
		return false;
	}

	m_Renderer = SDL_CreateRenderer(
		static_cast<SDL_Window*>(m_WindowHandle),
		-1,
		SDL_RENDERER_ACCELERATED
	);

	m_Texture = SDL_CreateTexture(
		static_cast<SDL_Renderer*>(m_Renderer),
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		width, height
	);

	SDL_SetRelativeMouseMode(SDL_TRUE);

	m_Width = width;
	m_Height = height;

	return true;
}

void Window::Destroy()
{
	SDL_DestroyTexture(static_cast<SDL_Texture*>(m_Texture));
	SDL_DestroyRenderer(static_cast<SDL_Renderer*>(m_Renderer));
	SDL_DestroyWindow(static_cast<SDL_Window*>(m_WindowHandle));
	SDL_Quit();
}

void Window::Render(const uint32_t* frameBuffer)
{
	SDL_UpdateTexture(
		static_cast<SDL_Texture*>(m_Texture),
		nullptr,
		frameBuffer,
		m_Width * sizeof(uint32_t)
	);

	SDL_RenderClear(static_cast<SDL_Renderer*>(m_Renderer));
	SDL_RenderCopy(
		static_cast<SDL_Renderer*>(m_Renderer),
		static_cast<SDL_Texture*>(m_Texture),
		nullptr, nullptr
	);
	SDL_RenderPresent(static_cast<SDL_Renderer*>(m_Renderer));
}

void Window::PollEvents(bool& running, std::vector<Event>& outEvents)
{
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		Event event;
		switch (e.type)
		{
		case SDL_QUIT:
			running = false;
			event.type = Event::Type::QUIT;
			outEvents.push_back(event);
			break;
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			event.type = (e.type == SDL_KEYDOWN) ? Event::Type::KEYDOWN : Event::Type::KEYUP;
			switch (e.key.keysym.sym)
			{
			case SDLK_w: event.key = Key::W; break;
			case SDLK_a: event.key = Key::A; break;
			case SDLK_s: event.key = Key::S; break;
			case SDLK_d: event.key = Key::D; break;
			case SDLK_ESCAPE: event.key = Key::ESCAPE; break;
			case SDLK_LSHIFT:
			case SDLK_RSHIFT: event.key = Key::SHIFT; break;
			case SDLK_SPACE: event.key = Key::SPACE; break;
			default: event.key = Key::UNKNOWN; break;
			}
			outEvents.push_back(event);
			break;
		case SDL_MOUSEMOTION:
			event.type = Event::Type::MOUSEMOVE;
			event.mouseX = e.motion.x;
			event.mouseY = e.motion.y;
			event.mouseMotionX = e.motion.xrel;
			event.mouseMotionY = e.motion.yrel;
			outEvents.push_back(event);
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			event.type = (e.type == SDL_MOUSEBUTTONDOWN) ? Event::Type::MOUSEBUTTONDOWN : Event::Type::MOUSEBUTTONUP;
			switch (e.button.button)
			{
			case 1:
				event.mouseButton = MouseButton::LEFT;
				break;
			case 2:
				event.mouseButton = MouseButton::MIDDLE;
				break;
			case 3:
				event.mouseButton = MouseButton::RIGHT;
				break;
			default:
				event.mouseButton = MouseButton::NONE;
				break;
			}
			outEvents.push_back(event);
			break;
		default:
			break;
		}
	}
}