#pragma once

#include "BaseModule.hpp"

namespace ENGINE::GRAPHICS
{
    struct DaVinci : ENGINE_MODULE_INTERFACE
    {
    public:
        DaVinci() : ENGINE_MODULE_INTERFACE("DaVinci")
        {
            Parent_Window_ptr = nullptr;
        }
        
        
        
        bool New_Parent_Window(std::string, int w, int h);

        bool Initialise_Context();

        
        
        enum E_EventType : short
        {
            NOTHING,
            QUIT,
        };

        E_EventType Check_For_Events();

        void* Get_Parent_Window_ptr()
        {
            return Parent_Window_ptr;
        }

    private:
        void* Parent_Window_ptr;
    };
}
