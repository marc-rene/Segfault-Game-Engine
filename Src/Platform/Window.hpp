#pragma once

#include "SDL3/SDL.h"
#include "SDL3/SDL_timer.h"
#include "SDL3/SDL_keyboard.h"

#include "../Resources/EngineSettings.hpp"

#define WINDOW_MANAGER_NAME "ENGINE::WindowManager"


namespace ENGINE::Settings
{
	struct WindowSettings : public PrimativeSettings
	{
		std::string DefaultWindowTitle = "HAVE WE SEGFAULTED YET?";


		SDL_WindowFlags SDLFlag_windowProperties = SDL_WINDOW_RESIZABLE;
		int int_preferredWindowWidth = 1280;
		int int_preferredWindowHeight = 720;

		bool SetSettingsFromConfig() override
		{
			int_preferredWindowWidth = ENGINE::Platform::FileIO::Config::GetSetting_int("Window", "Width");
			int_preferredWindowHeight = ENGINE::Platform::FileIO::Config::GetSetting_int("Window", "Height");

			return true;
		}
	};
}


namespace ENGINE::Platform
{
	class WindowManager
	{
	public:

		bool Initialise();
		SDL_Window* CreateMainWindow(ENGINE::Settings::WindowSettings* p_windowProperties, bool p_Force = false);
		SDL_Window* GetMainWindowRef();

		std::string* GetWindowTitle()
		{
			return &m_windowSettings.DefaultWindowTitle;
		}

		WindowManager();

	private:
		bool b_isInitialised = false;
		inline static SDL_Window* m_MainSDLWindow;
		ENGINE::Settings::WindowSettings m_windowSettings;

	};// WindowManager
}