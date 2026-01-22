// ReSharper disable CppInconsistentNaming
#pragma once

#include "BaseModule.hpp"
#define WIN32_LEAN_AND_MEAN 

#include <windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <shellapi.h>
#include <algorithm>
#include <cassert>
#include <chrono>


namespace ENGINE::GRAPHICS
{
    using namespace Microsoft::WRL;

    struct DaVinci : ENGINE_MODULE_INTERFACE
    {
    public:
        // Instantiate all our vars, using inherited constructor
        DaVinci();


        /// Try make a new window using SDL_CreateWindow
        /// @param Parent_Window_Name What to call it? we'll use c_str() on it 
        /// @param initial_width 1600px wide by default
        /// @param initial_height 900px high by default
        /// @return Success?
        bool New_Parent_Window(std::string Parent_Window_Name, int initial_width = 1600, int initial_height = 900);


        /// Let's actually get our directX context and data up and running!
        /// @param use_default Use Default settings or load config vars? TODO: Implement config vars
        /// @return Success?
        bool Initialise_Context(bool use_default);


        // Stand-in for SDL_EventType because we don't want SDL include hell
        enum E_EventType : short
        {
            NOTHING,
            QUIT,
        };

        // Any Window Resize? KeyDown? whut?
        E_EventType Check_For_Events();


        // Parent_Window_ptr is ACTUALLY a SDL_Window*... but let's get it as a void*
        void* Get_Parent_Window_ptr() const;


        // Parent_Window_ptr is ACTUALLY a SDL_Window*... but let's get it as a HWND
        HWND Get_Parent_Window_HWND();


        /// Get the width of our parent window... returns null if it's invalid
        /// @param width pointer to where we'll store the value of the width
        /// @param height same but for the height
        /// @return Is there even a window?
        bool Get_Window_Size(int* width, int* height);


        // Do this BEFORE we make a D3D12 device
        bool EnableGraphicsDebugLayer();

        
        // Cycle through all GPU's on the system and select the best most capable one!
        ComPtr<IDXGIAdapter4> Get_Best_Graphics_Adapter();


        // Time to actually make something that can allocate our textures and fences and heaps and stuff
        ComPtr<ID3D12Device2> Create_Device(ComPtr<IDXGIAdapter4> which_adapter_to_use);


        /// When we want to send a series of commands to the Graphics card DX12 style
        /// @param device What DX12 Device do we want to do the work?
        /// @param type "Copy" between CPU <-> GPU  "Compute" can copy and compute  "Direct" Just does everything
        ComPtr<ID3D12CommandQueue> Create_Command_Queue(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type);
        
        
        // Do we support G-Sync or Freesync or something?
        bool Check_Variable_Refresh_Rate_Support();
        
        
        // Time to create our row of frames that will get presented to the screen one after the other
        ComPtr<IDXGISwapChain4> Create_Swap_Chain(HWND hWnd, ComPtr<ID3D12CommandQueue> commandQueue);
        
        
        
    private:
        void* Parent_Window_ptr;

        struct Render_Settings
        {
            struct Display_Settings
            {
                bool g_VSync = false;

                // TODO: Figure this out
                bool g_VariableRefreshRate = false;

                // Use Windowed by default
                bool g_Fullscreen = false;
            };

            struct Graphics_Settings
            {
            };
        };

        struct Pipeline_Objects
        {
            // TODO: Make this configuarable... 2 is good for fps, 3 is good for more smooth
            static constexpr UINT frames_in_flight = 2;

            // Pipeline objects
            inline static bool is_initialised = false;
            inline static bool harness_the_WARP = false;
            DXGI_ADAPTER_DESC1 g_GraphicsAdapterDescription;

            ComPtr<ID3D12Fence> g_Fence;
            uint64_t g_FenceValue = 0;
            uint64_t g_FrameFenceValues[frames_in_flight] = {};
            HANDLE g_FenceEvent;

            ComPtr<ID3D12Device2> g_Device;
            ComPtr<ID3D12Debug> g_DebugInterface;
            ComPtr<ID3D12CommandQueue> g_CommandQueue;
            ComPtr<IDXGISwapChain4> g_SwapChain;
            ComPtr<ID3D12Resource> g_BackBuffers[frames_in_flight];
            ComPtr<ID3D12GraphicsCommandList> g_CommandList;
            ComPtr<ID3D12CommandAllocator> g_CommandAllocators[frames_in_flight];
            ComPtr<ID3D12DescriptorHeap> g_RTVDescriptorHeap;
            UINT g_RTVDescriptorSize;
            UINT g_CurrentBackBufferIndex;
        };

        Pipeline_Objects g_Pipeline_Objects;
    };
}
