#include "../Include/DaVinci.hpp"

#include <SDL3/SDL_main.h>

#include "SDL3/SDL_video.h"


bool ENGINE::GRAPHICS::DaVinci::New_Parent_Window(std::string name, int w, int h)
{
    Parent_Window_ptr = SDL_CreateWindow(name.c_str(), w, h, SDL_WINDOW_MOUSE_FOCUS);
    
    if (Parent_Window_ptr == NULL)
    {
        WARN(Get_Module_Name(), "Could't create SDL window because {}", SDL_GetError());
        return false;
    }
    return true;
}

bool ENGINE::GRAPHICS::DaVinci::Initialise_Context()
{
    return SDL_Init(SDL_INIT_VIDEO);
}

ENGINE::GRAPHICS::DaVinci::E_EventType ENGINE::GRAPHICS::DaVinci::Check_For_Events()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
        {
            return E_EventType::QUIT;
        }
    }
    return E_EventType::NOTHING;
}
