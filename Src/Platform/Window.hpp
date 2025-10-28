#pragma once

#include "SDL3/SDL.h"
#include "SDL3/SDL_timer.h"
#include "SDL3/SDL_keyboard.h"
#include "SDL3/SDL_mouse.h"

#include "DirectXMath.h"
#include "../Resources/EngineSettings.hpp"


namespace ENGINE::Platform
{
	class WindowManager
	{
	public:

		bool Initialise();
		SDL_Window* CreateMainWindow(bool p_Force = false);
		SDL_Window* GetMainWindowRef();

		static mint PollEvents();

		std::string* GetWindowTitle()
		{
			namespace S = ENGINE::Settings;
			return S::ActiveSettings::GetSetting_str(S::E_Settings::WINDOW_TITLE);
		}

		WindowManager(const WindowManager& obj) = delete;

		inline static WindowManager* GetInstance()
		{
			if (self_reference == nullptr) {
				std::lock_guard<std::mutex> lock(mtx);
				if (self_reference == nullptr) {
					self_reference = new WindowManager();
				}
			}
			return self_reference;
		}

		inline static const DirectX::XMFLOAT2* GetMouseWindowPosition()
		{
			return &ENGINE::Platform::WindowManager::GetInstance()->Mouse_pos;
		}

		inline static const float* GetMouseWindowPosition_X()
		{
			return &ENGINE::Platform::WindowManager::GetInstance()->Mouse_pos.x;
		}

		inline static const float* GetMouseWindowPosition_Y()
		{
			return &ENGINE::Platform::WindowManager::GetInstance()->Mouse_pos.y;
		}

	private:
		DirectX::XMFLOAT2 Mouse_pos = DirectX::XMFLOAT2(0.0f, 0.0f);

		bool b_isInitialised = false;
		inline static SDL_Window* m_MainSDLWindow;
		inline static WindowManager* self_reference;
		inline static std::mutex mtx;
		
		WindowManager() 
		{
			Initialise();
		};

	};// WindowManager
}