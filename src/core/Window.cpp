#include "Window.h"

#include <SDL.h>

#include "Log.h"

namespace Core
{
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
			SDL_WINDOW_RESIZABLE
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
			if (e.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				m_Width = e.window.data1;
				m_Height = e.window.data2;
				event.type = Event::Type::WindowResize;
				outEvents.push_back(event);
				m_Texture = SDL_CreateTexture(
					static_cast<SDL_Renderer*>(m_Renderer),
					SDL_PIXELFORMAT_ARGB8888,
					SDL_TEXTUREACCESS_STREAMING,
					m_Width, m_Height
				);
				continue;
			}

			switch (e.type)
			{
			case SDL_QUIT:
				running = false;
				event.type = Event::Type::Quit;
				outEvents.push_back(event);
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				event.type = (e.type == SDL_KEYDOWN) ? Event::Type::Keydown : Event::Type::Keyup;
				switch (e.key.keysym.sym)
				{
				case SDLK_w: event.key = Key::W; break;
				case SDLK_a: event.key = Key::A; break;
				case SDLK_s: event.key = Key::S; break;
				case SDLK_d: event.key = Key::D; break;
				case SDLK_ESCAPE: event.key = Key::Escape; break;
				case SDLK_LSHIFT:
				case SDLK_RSHIFT: event.key = Key::Shift; break;
				case SDLK_RCTRL: event.key = Key::RightCtrl; break;
				case SDLK_LCTRL: event.key = Key::LeftCtrl; break;
				case SDLK_SPACE: event.key = Key::Space; break;
				case SDLK_e: event.key = Key::E; break;
				case SDLK_t: event.key = Key::T; break;
				case SDLK_q: event.key = Key::Q; break;
				case SDLK_0: event.key = Key::K_0; break;
				case SDLK_1: event.key = Key::K_1; break;
				case SDLK_2: event.key = Key::K_2; break;
				case SDLK_3: event.key = Key::K_3; break;
				case SDLK_4: event.key = Key::K_4; break;
				case SDLK_5: event.key = Key::K_5; break;
				case SDLK_6: event.key = Key::K_6; break;
				case SDLK_7: event.key = Key::K_7; break;
				case SDLK_8: event.key = Key::K_8; break;
				case SDLK_9: event.key = Key::K_9; break;
				default: event.key = Key::Unknown; break;
				}
				outEvents.push_back(event);
				break;
			case SDL_MOUSEMOTION:
				event.type = Event::Type::MouseMove;
				event.mouseX = e.motion.x;
				event.mouseY = e.motion.y;
				event.mouseMotionX = e.motion.xrel;
				event.mouseMotionY = e.motion.yrel;
				outEvents.push_back(event);
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				event.type = (e.type == SDL_MOUSEBUTTONDOWN) ? Event::Type::MouseButtonDown : Event::Type::MouseButtonUp;
				switch (e.button.button)
				{
				case 1:
					event.mouseButton = MouseButton::Left;
					break;
				case 2:
					event.mouseButton = MouseButton::Middle;
					break;
				case 3:
					event.mouseButton = MouseButton::Right;
					break;
				default:
					event.mouseButton = MouseButton::None;
					break;
				}
				outEvents.push_back(event);
				break;
			case SDL_MOUSEWHEEL:
				event.type = (e.wheel.y > 0) ? Event::Type::MouseWheelDown : Event::Type::MouseWheelUp;
				event.mouseButton = MouseButton::Middle;
				event.wheelValue = e.wheel.y;
				outEvents.push_back(event);
				break;
			default:
				break;
			}
		}
	}

	void Window::SetMouseMode(MouseMode state)
	{
		if (state == MouseMode::Captured)
			SDL_SetRelativeMouseMode(SDL_TRUE);
		else
			SDL_SetRelativeMouseMode(SDL_FALSE);
	}
}
