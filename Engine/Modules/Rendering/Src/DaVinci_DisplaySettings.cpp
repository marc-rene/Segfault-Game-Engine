#include "../Include/DaVinci.hpp"

#define STRINGISE(x) L##x       // Do NOT use this
#define WIDEN(x) STRINGISE(x)   // DO use this to make String a Wide string

namespace ENGINE::GRAPHICS
{
    struct DaVinci::Display_Settings
    {
        std::wstring Window_Title       =   WIDEN(ENGINE_NAME);
        
        unsigned short Window_Height    =   600; 
        unsigned short Window_Width     =   800;
        
        // Fullscreen? Resizable? 
        int Flags = 0;
        
        Display_Settings()
        {
            Window_Title =   WIDEN(ENGINE_NAME);
            Window_Height =   600;
            Window_Width =   800;
            Flags;
        }
    };
}
