// ReSharper disable CppInconsistentNaming
#pragma once

#ifdef DIRECTX_11_SUPPORT
#include "DirectX_11_impl.hpp"
#endif

#ifdef DIRECTX_12_SUPPORT
#include "DirectX_12_impl.hpp"
#endif

#ifdef VULKAN_SUPPORT
#include "Vulkan_impl.hpp"
#endif

#ifdef WEBGPU_SUPPORT
#include "WebGPU_impl.hpp"
#endif


#include "BaseModule.hpp"
#include "Definitions_Global.hpp"
#include "Definitions_Display.hpp"


namespace ENGINE::GRAPHICS
{
    // All info regarding the 'WINDOW' that we see... it's size? title? resizable? etc...
    struct Display_Info;


    struct DaVinci : ENGINE_MODULE_INTERFACE
    {
    public:
        // Instantiate all our vars, using inherited constructor
        DaVinci();


        // Any Window Resize? KeyDown? Return a bool if there's anything we need to do
        bool Poll_for_Events();


        // Return a vector* of all events that happened this frame that need to be processed
        std::vector<E_DaVinci_WindowEventType>* Get_All_Window_Events_this_Frame();


        /// Try to make a new window using SDL_CreateWindow
        /// @param Parent_Window_Name What do you want to call this Window? 
        /// @param Initial_Dimmensions An Int[2] of the [Width, Height] of the window
        /// @param Window_Flags Want it to be Resizable? Borderless? Start Maximised? check out Definitions_Display.hpp
        /// @return False == Something fucked up
        bool New_Parent_Window(std::string Parent_Window_Name, DaVinci_WindowDimensions Initial_Dimmensions,
                               DaVinci_WindowFlags Window_Flags = 0);


        // Try to make a window using SDL_CreateWindow, but we're using Display_Info instead for more controll 
        bool New_Parent_Window(Display_Info* new_window_parameters);


        /// Try to initialise our Device using either Vulkan (WIP), DirectX 11 (WIP), or DirectX 12... 
        /// @param Preferred_RHI What RHI would you like to use? Only DirectX 12 is supported for now
        /// @return Hopefully not false
        bool Initialise_Render_Devices(E_DaVinci_RenderAPI Preferred_RHI = E_DaVinci_RenderAPI::DirectX_12);


        /// Make our swap chains, RTV's, Callbacks, Everything! Our Pipeline objects about to get full
        /// @return Hopefully not false
        bool Initialise_Render_Resources();

        
        /// The content scale is the expected scale for content based on the DPI settings of the display. 
        /// A 4K display might have a 2.0 (200%) display scale, which means that the user expects UI 
        /// elements to be twice as big on this display.
        /// @return -1.0 on Failure  |  > 1.0 on success
        float Get_Window_Content_Scale();


        /// Get the Power setting that we're running at:                        \n
        /// 1: Rendering has paused                                             \n
        /// 2: Rendering + GPU is at reduced power, 10-30fps limit maybe?       \n
        /// 3: Rendering is at full blast! Aim for as high as V-Sync and beyond!
        /// @return 0 == Failure 
        uMint Get_Power_Setting();
        
        
        int Get_Window_Width();
        int Get_Window_Height();
        
        /// @param width_value_ptr we'll write the width of the window to this pointer
        void Get_Window_Width(uInt_32* width_value_ptr);
        
        /// @param height_value_ptr we'll write the height of the window to this pointer
        void Get_Window_Height(uInt_32* height_value_ptr);
        
        /// @param size_value_ptr we'll write the wdith + height of the window to this int[2] pointer
        void Get_Window_Dimensions(DaVinci_WindowDimensions* size_value_ptr);

        
        /// Do we want to enable a debug layer for our Renderer?
        /// Do this BEFORE calling Initialise_Render_Devices()
        /// @return if you called this AFTER you already made your RHI device... You'll probably get false \n
        /// Try call Attempt_Render_Device_Reinitialisation()
        bool Enable_Graphics_Debug_Layer();
        
        
        // Something REALLY bad happened, and we need to TRY remake our Render device... I hope this NEVER gets called
        bool Attempt_Render_Device_Reinitialisation();
        
        
        // When the Window gets resized we need to update the swapchain and everything
        bool On_Window_Resize();
        
        
        // Remember, because Windows is funky, it'll only ever be Fullscreen windowed borderless
        bool Set_Window_Fullscreen();
        
        
        // Set to a new Size... ideally what was ever used before
        bool Set_Window_Windowed();
        
        
        // Resize the Window to a new size... will call On_Window_Resize()
        bool Set_Window_Size(DaVinci_WindowDimensions New_Size);
        
        
        // Update our swapchain, current back-buffer index, etc...
        bool Before_Show_Next_Frame();
        
        
        // Try to show the next frame in our swapchain
        bool Show_Next_Frame();

        
        // Try to calculate deltatime and all that good stuff, get ready for the next frame
        bool After_Shown_Frame();
        
        
        // What colour will we set the background when nothing's been rendered?
        void Set_Clear_Colour(float R, float G, float B, float A);
        
        
        
    private:
        inline static DaVinci* self_ptr;

        E_DaVinci_RenderAPI current_render_RHI;


#ifdef DIRECTX_11_SUPPORT
        std::shared_ptr<D3D11_PipelineObjects> g_D3D11_pipelineObjects;
#endif

        
#ifdef DIRECTX_12_SUPPORT
        std::shared_ptr<D3D12_PipelineObjects> g_D3D12_pipelineObjects;
#endif

        
#ifdef VULKAN_SUPPORT
        std::shared_ptr<VULKAN_PipelineObjects> VULKAN_pipelineObjects;
#endif

        
#ifdef WEBGPU_SUPPORT
        std::shared_ptr<WEBGPU_PipelineObjects> g_WEBGPU_pipelineObjects;
#endif

        
        std::vector<E_DaVinci_WindowEventType> all_events_this_frame;

        
        Display_Info* parent_window;

        
        // Start time of current frame
        inline static std::chrono::time_point<std::chrono::steady_clock>
        start_frame_render_time = std::chrono::high_resolution_clock::now();

        
        // End time of current frame
        inline static std::chrono::time_point<std::chrono::steady_clock>
        end_frame_render_time = std::chrono::high_resolution_clock::now();
    };
}
