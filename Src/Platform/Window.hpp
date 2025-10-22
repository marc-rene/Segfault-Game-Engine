#pragma once

#include "SDL3/SDL.h"

#include "../Resources/EngineSettings.hpp"

#define WINDOW_MANAGER_NAME "ENGINE::WindowManager"


namespace ENGINE::Settings
{
	struct WindowSettings : public PrimativeSettings
	{
		std::string DefaultWindowTitle = "HAVE WE SEGFAULTED YET?";


		SDL_WindowFlags SDLFlag_windowProperties;
		int int_preferredWindowWidth = 1280;
		int int_preferredWindowHeight = 720;

		


		bool GetValue_bool(E_Settings SettingID) override
		{
			switch (SettingID)
			{
			case WINDOW_IS_FULLSCREEN:
				return (SDLFlag_windowProperties & SDL_WINDOW_FULLSCREEN);
			case WINDOW_IS_BORDERLESS:
				return (SDLFlag_windowProperties & SDL_WINDOW_BORDERLESS);
			case WINDOW_IS_RESIZABLE:
				return (SDLFlag_windowProperties & SDL_WINDOW_RESIZABLE);
			case WINDOW_IS_MINIMISED:
				return (SDLFlag_windowProperties & SDL_WINDOW_MINIMIZED);
			case WINDOW_IS_MAXIMISED:
				return (SDLFlag_windowProperties & SDL_WINDOW_MAXIMIZED);
			case WINDOW_IS_ALWAYS_ON_TOP:
				return (SDLFlag_windowProperties & SDL_WINDOW_ALWAYS_ON_TOP);
			default:
				WARNc("Window settings has no idea (E_Settings enum type) is");
				return false;
			}
		}

		uFatty* GetValue_int(E_Settings SettingID) override
		{
			switch (SettingID)
			{
			case WINDOW_PREFERRED_WIDTH:
				return reinterpret_cast<uFatty*>(&int_preferredWindowWidth);

			case WINDOW_PREFERRED_HEIGHT:
				return reinterpret_cast<uFatty*>(&int_preferredWindowHeight);
			}
		}

		std::string* GetValue_str(E_Settings SettingID) override
		{
			switch (SettingID)
			{
			case WINDOW_TITLE:
				return &DefaultWindowTitle;
			default:
				static std::string invalidName = "Invalid Name";
				return &invalidName;
			}
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
		SDL_Window* m_MainSDLWindow;
		ENGINE::Settings::WindowSettings m_windowSettings;

	};// WindowManager
}