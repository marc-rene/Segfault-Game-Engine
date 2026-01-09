#pragma once

/*
#include "SDL3/SDL.h"
#include "SDL3/SDL_timer.h"
#include "SDL3/SDL_keyboard.h"
#include "SDL3/SDL_mouse.h"
#include "SDL3/SDL_properties.h"

#include <Windows.h>
#include "DirectXMath.h"
#include "../Core/BaseModule.hpp"
#include "../Resources/EngineSettings.hpp"

namespace ENGINE
{
    struct DaVinci : public ENGINE_MODULE_INTERFACE
    {
        static DaVinci* Get_Instance()
        {
            if (m_self_ptr == nullptr)
            {
                creation_mtx.lock();
                m_self_ptr = new DaVinci();
            }
        }

    protected:
        std::string_view m_module_title = "DaVinci";

        DaVinci()
        {
            Info("DaVinci is Initialised! Now we just need to make some DXGI stuff");
        };

        static std::mutex creation_mtx;
        static DaVinci* m_self_ptr;
    };
}






// DEPRECATED - MOVING TO DAVINCI CLASS FOR ALL RENDERING
namespace ENGINE::Platform
{
    class WindowManager
    {
    public:

        bool Initialise();
        SDL_Window* CreateMainWindow(bool p_Force = false);
        SDL_Window* GetMainWindowRef();
        HWND GetMainWindowHWND();

        static mint PollEvents();

        //HWND GetWindowHandle_Windows()

        bool RefreshWindowTitle();



        bool SetWindowTitle(std::string new_title, bool UpdateActiveWindowTitleToo = false);



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

*/


