#include "../Include/DaVinci.hpp"
#include <utility>

#include "../Include/DaVinci.hpp"


namespace ENGINE::GRAPHICS
{
    struct Display_Info
    {
        std::wstring Window_Title = TO_WIDE_STRING(ENGINE_NAME);

        DaVinci_WindowDimensions Window_Size = {800, 600};

        // Fullscreen? Resizable? 
        DaVinci_WindowFlags Flags = 0;

        
        
        
        
        Display_Info()
        {
            Window_Title = TO_WIDE_STRING(ENGINE_NAME);
            Window_Size[0] =  800;
            Window_Size[1] =  600;
            Flags = DAVINCI_WINDOW__RESIZABLE;
        }
        
        Display_Info(   std::wstring new_window_title,   uInt_32 initial_width, 
                        uInt_32 initial_height,         DaVinci_WindowFlags initial_flags) :
        Window_Title(std::move(new_window_title)), 
        Window_Size(initial_width, initial_height), 
        Flags(initial_flags)
        {
            Window_Title = TO_WIDE_STRING(ENGINE_NAME);
            Window_Size[0] =  800;
            Window_Size[1] =  600;
            Flags = DAVINCI_WINDOW__RESIZABLE;
        }
    };
}
