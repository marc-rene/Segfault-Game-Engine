#pragma once

#include "../Platform/Window.hpp"
#include "../Rendering/Rendering.hpp"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_dx11.h" 

// TODO: Figure out Directx 12 support
//#include "imgui_impl_dx12.h" 



namespace ENGINE
{
    struct Editor
    {
        const char* TITLE = "Segfault Editor";

        void OnFrameStart();

        void ComputeDrawData();
        void RenderDrawData();


        bool ProcessSDLEvent(SDL_Event* event)
        {
            return ImGui_ImplSDL3_ProcessEvent(event);
        }



        inline static bool Is_Initialised()
        {
            return b_initialised;
        }

        inline static Editor* GetInstance()
        {
            if (self_ptr == nullptr)
                self_ptr = new Editor();

            return self_ptr;
        }


        struct Windows;

    private:
        inline static bool b_initialised = false;
        inline static Editor* self_ptr;

        Editor();

    };


}
