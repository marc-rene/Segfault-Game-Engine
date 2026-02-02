// ReSharper disable CppInconsistentNaming
#pragma once


#include "BaseModule.hpp"


namespace ENGINE::GRAPHICS
{
    struct DaVinci : ENGINE_MODULE_INTERFACE
    {
    public:
        // Instantiate all our vars, using inherited constructor
        DaVinci();

        // All info regarding the 'WINDOW' that we see... it's size? title? resizable? etc...
        struct Display_Settings;

        /// Try make a new window using SDL_CreateWindow
        /// @param Parent_Window_Name What to call it? we'll use c_str() on it 
        /// @param initial_width 1600px wide by default
        /// @param initial_height 900px high by default
        /// @return Success?
        bool New_Parent_Window(std::string Parent_Window_Name, int initial_width = 1600, int initial_height = 900,
                               bool resizable = true);

        bool New_Parent_Window(Display_Settings new_window_parameters);


        
        //Display_Settings Active_Display_Settings = {NULL, NULL, NULL, NULL};
    
    private:
        inline static DaVinci* self_ptr;

        void* parent_window_ptr;

        inline static std::chrono::time_point<std::chrono::steady_clock>
        start_frame_render_time = std::chrono::high_resolution_clock::now();
        
        inline static std::chrono::time_point<std::chrono::steady_clock>
        end_frame_render_time = std::chrono::high_resolution_clock::now();
    };
}
