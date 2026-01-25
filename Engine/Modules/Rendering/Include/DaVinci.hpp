// ReSharper disable CppInconsistentNaming
#pragma once

#include "BaseModule.hpp"
#define WIN32_LEAN_AND_MEAN 

#include <windows.h>
#include <shellapi.h>

// The min/max macros conflict with like-named member functions.
// Only use std::min and std::max defined in <algorithm>.
#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif


// Windows Runtime Library. Needed for ComPtr<> template class.
#include <wrl.h>
using namespace Microsoft::WRL;


#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <algorithm>
#include <cassert>
#include <chrono>


namespace ENGINE::GRAPHICS
{
    struct Camera
    {
        float m_FoV;

        DirectX::XMMATRIX m_ModelMatrix;
        DirectX::XMMATRIX m_ViewMatrix;
        DirectX::XMMATRIX m_ProjectionMatrix;
    };


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
        bool New_Parent_Window(std::string Parent_Window_Name, int initial_width = 1600, int initial_height = 900,
                               bool resizable = true);


        /// Let's actually get our directX context and data up and running!
        /// @param use_default Use Default settings or load config vars? TODO: Implement config vars
        /// @return Success?
        bool Initialise_Context(bool use_default);


        float Get_Main_Display_Scale();


        // Stand-in for SDL_EventType because we don't want SDL include hell
        enum E_EventType : short
        {
            NOTHING = 0,
            QUIT,
            WINDOW_RESIZED,
            WINDOW_MOVED_TO_NEW_DISPLAY,
            WINDOW_IS_HIDDEN,
            WINDOW_IS_NO_LONGER_HIDDEN,
            LOW_MEMORY,
            ENTERED_BACKGROUND,
            ENTERED_FOREGROUND,
            DARK_MODE_THEME_CHANGED,

            DISPLAY_ADDED,
            DISPLAY_REMOVED,
            DISPLAY_HDR_STATUS_CHANGED,

            // --- Keyboard Events ---//
            KEY_DOWN,
            KEY_UP,
            KEYMAP_CHANGED,
            // -----------------------//

            // ---  Mouse Events   ---//
            MOUSE_ENTERED_WINDOW,
            MOUSE_HAS_MOVED,
            MOUSE_BUTTON_DOWN,
            MOUSE_BUTTON_UP,
            MOUSE_WHEEL_MOVED,
            // -----------------------//

            // --- Gamepad Events --- //
            GAMEPAD_AXIS_MOTION,
            GAMEPAD_BUTTON_DOWN,
            GAMEPAD_BUTTON_UP,
            GAMEPAD_ADDED,
            GAMEPAD_REMOVED,
            GAMEPAD_REMAPPED,
            GAMEPAD_TOUCHPAD_DOWN,
            GAMEPAD_TOUCHPAD_MOTION,
            GAMEPAD_TOUCHPAD_UP,
            // -----------------------//

            // ---  Audio Events   ---//
            AUDIO_DEVICE_ADDED,
            AUDIO_DEVICE_REMOVED,
            AUDIO_DEVICE_FORMAT_CHANGED,
            // -----------------------//
        };

        // Any Window Resize? KeyDown? whut?
        E_EventType Check_For_Events();


        /// What power are we running our graphics card at?
        /// 
        ///     0 - Don't render... pause frame
        /// 
        ///     1 - Render but maybe try turn on V-Sync or something to slow down the GPU
        ///     
        ///     2 - FULL POWER
        uint8_t Get_Power_Setting() const;


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
        bool Enable_Graphics_Debug_Layer();


        // Cycle through all GPU's on the system and select the best most capable one!
        ComPtr<IDXGIAdapter4> Get_Best_Graphics_Adapter(bool use_software_renderer = false);


        // Time to actually make something that can allocate our textures and fences and heaps and stuff
        ComPtr<ID3D12Device2> Create_Device(ComPtr<IDXGIAdapter4> which_adapter_to_use);


        /// When we want to send a series of commands to the Graphics card DX12 style
        /// @param device What DX12 Device do we want to do the work?
        /// @param type "Copy" between CPU <-> GPU  "Compute" can copy and compute  "Direct" Just does everything
        ComPtr<ID3D12CommandQueue> Create_Command_Queue(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type);


        // Do we support G-Sync or Freesync or something?
        bool Check_Variable_Refresh_Rate_Support();


        // Time to create our row of frames that will get presented to the screen one after the other
        ComPtr<IDXGISwapChain4> Create_Swap_Chain(HWND hWnd, ComPtr<ID3D12CommandQueue> commandQueue,
                                                  uint32_t width, uint32_t height);


        /// Time to create an array of resource views... a Descriptor Heap
        /// @param device What Initialised DirectX 12 'device' will be in charge of the allocation?
        /// @param type is it a Render Target View (RTV)? Depth Stencil View (DSV)? Sampler? 
        ///     Combo of Const-Buffer, Shader-Resource, and Unordered-Access Views (CBV_SRV_UAV)?

        /// @param numDescriptors How many Items "Descriptors" in this array? 
        /// @return HOPEFULLY not a nullptr
        ComPtr<ID3D12DescriptorHeap> Create_Descriptor_Heap(ComPtr<ID3D12Device2> device,
                                                            D3D12_DESCRIPTOR_HEAP_TYPE type,
                                                            uint32_t numDescriptors);


        /// For each back buffer of the swap chain, a single Render-Target-View is used to describe the resource
        /// @param device Who's in charge of memory allocations?
        /// @param swapChain Which one will we use?
        /// @param descriptorHeap Which array are we using to hold our buffers/resources?
        /// @return If all goes well... `true`! Only return `false` if we couldn't find one of the back-buffers
        bool Update_Render_Target_Views(ComPtr<ID3D12Device2> device, ComPtr<IDXGISwapChain4> swapChain,
                                        ComPtr<ID3D12DescriptorHeap> descriptorHeap);


        /// A command allocator is the backing memory used by a command list. A command allocator can only 
        /// be reset after the commands recorded in the command list have finished executing on the GPU
        /// @param device Who's in charge of memory allocations?
        /// @param type "Copy" between CPU <-> GPU  "Compute" can copy and compute  "Direct" Just does everything
        /// @return Hopefully not a `nullptr`!
        ComPtr<ID3D12CommandAllocator> Create_Command_Allocator(ComPtr<ID3D12Device2> device,
                                                                D3D12_COMMAND_LIST_TYPE type);


        /// A Command List is used for recording commands that are executed on the GPU. Unlike the command allocator, 
        /// the command list can be reused immediately after it has been executed on the command queue. 
        /// The only restriction is that the command list must be reset first before recording any new commands.
        /// @param device Who's in charge of memory allocations?
        /// @param commandAllocator Who's organising this? Use Create_Command_Allocator(device, type) to make this
        /// @param type "Copy" between CPU <-> GPU  "Compute" can copy and compute  "Direct" Just does everything
        /// @return Hopefully not a `nullptr`!
        ComPtr<ID3D12GraphicsCommandList> Create_Command_List(ComPtr<ID3D12Device2> device,
                                                              ComPtr<ID3D12CommandAllocator> commandAllocator,
                                                              D3D12_COMMAND_LIST_TYPE type);


        /// A Fence is going to keep everyone in sync, in check!
        /// @param device Who's the GPU allocation boss?
        /// @return Hopefully not a `nullptr`!
        ComPtr<ID3D12Fence> Create_Fence(ComPtr<ID3D12Device2> device);


        /// If the fence has not yet been signaled with specific value, the CPU thread will need to block any 
        /// further processing until the fence has been signaled with that value.
        /// The CreateEventHandle function described next is used to create the OS event.
        /// @return Hopefully not NULL 
        HANDLE Create_Event_Handle();


        /// The Signal function is used to signal the fence from the GPU. The fence is only signaled 
        /// once the GPU command queue has reached that point during execution. Any commands that have been queued 
        /// before the signal method was invoked must complete execution FIRST!
        /// @param commandQueue 
        /// @param fence 
        /// @param fenceValue 
        /// @return 
        uint64_t Signal(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, uint64_t& fenceValue);


        /// It's possible that the CPU thread will need to stall to wait for the GPU queue to finish executing 
        /// commands that write to resources before being reused. Eg: before reusing a swap chain’s back buffer 
        /// resource, any commands that are using that resource as a render target must be complete before that 
        /// back buffer resource can be reused.
        /// @param fence 
        /// @param fenceValue 
        /// @param fenceEvent 
        /// @param duration 
        void Wait_For_Fence_Value(ComPtr<ID3D12Fence> fence,
                                  uint64_t fenceValue,
                                  HANDLE fenceEvent,
                                  std::chrono::milliseconds duration = std::chrono::milliseconds::max()
        );


        /// Make sure that any commands previously executed on the GPU have finished executing before the CPU thread 
        /// is allowed to continue processing.
        void Flush(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence,
                   uint64_t& fenceValue, HANDLE fenceEvent);


        // At the moment we just get the frame-rate
        void Update();


        // At the moment we ONLY 1: Clear back buffer; 2: Present Rendered Frame
        void Render();


        // This should be triggered by Check_For_Events()
        void Resize();


        // True: LETS GO BORDERLESS FULLSCREEN YAY!    False: Nah go back to window
        void Set_FullScreen(bool use_fullscreen);

        // Finish up last bits after our frame's rendered! Mostly DT calculations
        void Finish_Frame();


        // Gets a Window... finds the name of it... that's it
        static std::string Get_Window_Name_From_HWND(HWND hWnd);


        // This is more so for our CommandQueue and NitPick editor...
        inline static DaVinci* Get_Instance()
        {
            return self_ptr;
        }


        inline static void Set_New_DaVinci_Instance(DaVinci* new_instance_ptr)
        {
            self_ptr = new_instance_ptr;
        }


        /// Set the colour of the background when nothing is rendered
        /// @param red red amount (0-1)
        /// @param green green amount (0-1)
        /// @param blue blue amount (0-1)
        /// @param alpha ngl I'm currious what happens when you have a transparent clear colour?
        void Set_Clear_Colour(float red, float green, float blue, float alpha);


        ComPtr<ID3D12Device2> Get_Active_Device() const;

        ComPtr<ID3D12CommandQueue> Get_Active_CommandQueue() const;

        int Get_Number_of_Frames_In_Flight() const;

        DXGI_FORMAT Get_RTV_Frame_Buffer_Format() const;

        DXGI_FORMAT Get_Depth_Stencil_View_Frame_Buffer_Format() const;


        // Depth buffer.
        ComPtr<ID3D12Resource> Depth_Buffer;

        // Descriptor heap for depth buffer.
        ComPtr<ID3D12DescriptorHeap> Depth_Stencil_View_Heap;

        // Root signature
        ComPtr<ID3D12RootSignature> Root_Signature;

        // Pipeline state object.
        ComPtr<ID3D12PipelineState> Pipeline_State;

        /// We need to be able to make a "Resource" that can hold all vertex/indicies data
        /// until it can get sent to the GPU
        /// TODO: Make better comment for this
        /// @param commandList required to transfer the buffer data to the destination resource
        /// @param pDestinationResource  pointer to the destination resources created from this method 
        /// @param pIntermediateResource  pointer to the intermediate resources created from this method
        /// @param numElements the CPU buffer data that is transferred to the GPU resource
        /// @param elementSize the CPU buffer data that is transferred to the GPU resource
        /// @param bufferData the CPU buffer data that is transferred to the GPU resource
        /// @param flags Any extra flags that are needed before we render this?
        /// @return Success?
        bool Update_Buffer_Resource(
            ComPtr<ID3D12GraphicsCommandList2> commandList,
            ID3D12Resource** pDestinationResource,
            ID3D12Resource** pIntermediateResource,
            size_t numElements, size_t elementSize, const void* bufferData,
            D3D12_RESOURCE_FLAGS flags);

        
        
        void Resize_Depth_Buffer(int width, int height);
        
        
        
        
        Camera Primary_Camera = {45.0f};

        
    private:
        inline static DaVinci* self_ptr;

        void* Parent_Window_ptr;

        inline static std::chrono::time_point<std::chrono::steady_clock> start_frame_render_time =
            std::chrono::high_resolution_clock::now();
        inline static std::chrono::time_point<std::chrono::steady_clock> end_frame_render_time =
            std::chrono::high_resolution_clock::now();


        // Transition a resource to a particular state before using it. For example, 
        // before a texture can be used in a pixel shader, it must be transitioned to the PIXEL_SHADER_RESOURCE state.
        void transition_resource(ComPtr<ID3D12GraphicsCommandList2> commandList,
                                 ComPtr<ID3D12Resource> resource,
                                 D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);


        // Clear a render target view.
        void clear_rtv(ComPtr<ID3D12GraphicsCommandList2> commandList,
                       D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor);


        // Clear the depth of a depth-stencil view.
        void clear_depth(ComPtr<ID3D12GraphicsCommandList2> commandList,
                         D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth = 1.0f);


        struct Render_Settings
        {
            struct Display_Settings
            {
                inline static bool g_VSync = false;

                // TODO: Figure this out
                inline static bool g_VariableRefreshRate = false;

                // Use Windowed by default
                inline static bool g_Fullscreen = false;
            };

            struct Graphics_Settings
            {
            };
        };

        struct Pipeline_Objects
        {
            // TODO: Make this configuarable... 2 is good for fps, 3 is good for more smooth
            static constexpr UINT frames_in_flight = 2;
            inline static FLOAT clear_colour[4] = {0.4f, 0.6f, 0.9f, 1.0f};;

            /// What Power setting should we use?
            /// 
            ///     0 - Don't render... pause frame
            /// 
            ///     1 - Render but maybe try turn on V-Sync or something to slow down the GPU
            ///     
            ///     2 - FULL POWER
            inline static uint8_t power_mode = 2;


            // Pipeline objects
            inline static bool is_initialised = false;
            inline static bool is_first_frame = true;
            inline static bool harness_the_WARP = false;
            DXGI_ADAPTER_DESC1 g_GraphicsAdapterDescription;


            // DXGI_FORMAT_R8G8B8A8_UNORM: 8-bit... basically every SDR game... It's fast, but banding issues
            // DXGI_FORMAT_R10G10B10A2_UNORM: 10-bit... better gradients and HDR10-friendly, but we 
            //          need to correct the colour space and metadata, especially for HDR10!
            // DXGI_FORMAT_R16G16B16A16_FLOAT: 16-bit... BEST HDR, almost NO banding... but DAMN it's expensive
            inline static DXGI_FORMAT g_frameBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

            uint32_t g_FrameBufferWidth;
            uint32_t g_FrameBufferHeight;


            // DXGI_ALPHA_MODE_UNSPECIFIED: Transparency behavior is not specified.
            // DXGI_ALPHA_MODE_PREMULTIPLIED: Transparency behavior is premultiplied. 
            //          Each color is first scaled by the alpha value. The alpha value itself 
            //          is the same in both straight and premultiplied alpha. 
            //          Typically, no color channel value is greater than the alpha channel value. 
            //          If a color channel value in a premultiplied format is greater than the alpha channel, 
            //          the standard source-over blending math results in an additive blend.
            // DXGI_ALPHA_MODE_STRAIGHT: Indicates that the transparency behavior is not premultiplied. 
            //          The alpha channel indicates the transparency of the color.
            // DXGI_ALPHA_MODE_IGNORE: Indicates to ignore the transparency behavior.
            inline static DXGI_ALPHA_MODE g_frameAlphaBehaviour = DXGI_ALPHA_MODE_UNSPECIFIED;

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
