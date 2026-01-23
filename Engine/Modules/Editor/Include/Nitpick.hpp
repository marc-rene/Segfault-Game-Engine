#pragma once

#include "BaseModule.hpp"
#include "DaVinci.hpp"


namespace ENGINE::EDITOR
{
    struct Nitpick : ENGINE_MODULE_INTERFACE
    {
        Nitpick();

        inline static Nitpick* GetInstance()
        {
            return self_ptr;
        }

        static void Set_New_Editor_Instance(Nitpick* new_instance_ptr);

        bool Set_New_Host_DaVinci_Instance(GRAPHICS::DaVinci* new_host_davinci);

    private:
        inline static bool b_initialised = false;
        inline static Nitpick* self_ptr;
        inline static GRAPHICS::DaVinci* davinci_host;
    };
}
