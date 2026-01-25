#include "../Include/DaVinci.hpp"
#include "../Include/DaVinci.hpp"

#include <d3dx12.h>

#include <SDL3/SDL_init.h>
#include "SDL3/SDL_video.h"

using namespace DirectX;


ENGINE::GRAPHICS::DaVinci::DaVinci() : ENGINE_MODULE_INTERFACE("DaVinci")
{
    Parent_Window_ptr = nullptr;
}


void* ENGINE::GRAPHICS::DaVinci::Get_Parent_Window_ptr() const
{
    return Parent_Window_ptr;
}


HWND ENGINE::GRAPHICS::DaVinci::Get_Parent_Window_HWND()
{
    SDL_PropertiesID window_properties = SDL_GetWindowProperties(static_cast<SDL_Window*>(Get_Parent_Window_ptr()));

    HWND parent_window_hwnd = (HWND)SDL_GetPointerProperty(window_properties, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);

    if (FAILED(parent_window_hwnd))
    {
        Warn("For whatever reason, the HWND of parent window isnt valid");
        return NULL;
    }
    else
    {
        Trace(std::format("Correct HWND was found for window: {}", Get_Window_Name_From_HWND(parent_window_hwnd)));
    }
    return parent_window_hwnd;
}


bool ENGINE::GRAPHICS::DaVinci::New_Parent_Window(std::string name, int w, int h, bool allow_resize)
{
    Enable_Graphics_Debug_Layer();
    Check_Variable_Refresh_Rate_Support();

    SDL_WindowFlags sdl_window_flags = 0;
    sdl_window_flags |= SDL_WINDOW_MOUSE_FOCUS;
    sdl_window_flags |= SDL_WINDOW_RESIZABLE;
    //sdl_window_flags |= SDL_WINDOW_BORDERLESS;

    Parent_Window_ptr = SDL_CreateWindow(name.c_str(), w, h, sdl_window_flags);

    if (Parent_Window_ptr == NULL)
    {
        WARN(Get_Module_Name(), "Could't create SDL window because {}", SDL_GetError());
        return false;
    }
    g_Pipeline_Objects.g_FrameBufferWidth = w;
    g_Pipeline_Objects.g_FrameBufferHeight = h;


    ComPtr<IDXGIAdapter4> dxgiAdapter4 = Get_Best_Graphics_Adapter();


    // Initialise the GPU memory allocator boss
    g_Pipeline_Objects.g_Device = Create_Device(dxgiAdapter4);


    g_Pipeline_Objects.g_CommandQueue = Create_Command_Queue(g_Pipeline_Objects.g_Device,
                                                             D3D12_COMMAND_LIST_TYPE_DIRECT);


    g_Pipeline_Objects.g_SwapChain = Create_Swap_Chain(
        Get_Parent_Window_HWND(),
        g_Pipeline_Objects.g_CommandQueue,
        g_Pipeline_Objects.g_FrameBufferWidth,
        g_Pipeline_Objects.g_FrameBufferHeight);


    g_Pipeline_Objects.g_CurrentBackBufferIndex = g_Pipeline_Objects.g_SwapChain->GetCurrentBackBufferIndex();


    g_Pipeline_Objects.g_RTVDescriptorHeap = Create_Descriptor_Heap(
        g_Pipeline_Objects.g_Device,
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
        g_Pipeline_Objects.frames_in_flight);


    g_Pipeline_Objects.g_RTVDescriptorSize = g_Pipeline_Objects.g_Device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV);


    Update_Render_Target_Views(
        g_Pipeline_Objects.g_Device,
        g_Pipeline_Objects.g_SwapChain,
        g_Pipeline_Objects.g_RTVDescriptorHeap);


    for (int i = 0; i < g_Pipeline_Objects.frames_in_flight; ++i)
    {
        g_Pipeline_Objects.g_CommandAllocators[i] = Create_Command_Allocator(
            g_Pipeline_Objects.g_Device,
            D3D12_COMMAND_LIST_TYPE_DIRECT);
    }

    g_Pipeline_Objects.g_CommandList = Create_Command_List(
        g_Pipeline_Objects.g_Device,
        g_Pipeline_Objects.g_CommandAllocators[g_Pipeline_Objects.g_CurrentBackBufferIndex],
        D3D12_COMMAND_LIST_TYPE_DIRECT);


    g_Pipeline_Objects.g_Fence = Create_Fence(g_Pipeline_Objects.g_Device);
    g_Pipeline_Objects.g_FenceEvent = Create_Event_Handle();

    g_Pipeline_Objects.is_initialised = true;


    return true;
}


bool ENGINE::GRAPHICS::DaVinci::Initialise_Context(bool use_default)
{
    SDL_InitFlags init_flags = SDL_INIT_VIDEO;

    if (!use_default)
    {
        Error("Look, I know you don't want to use config vars, but we got a TODO for that... please wait");
    }

    return SDL_Init(SDL_INIT_VIDEO);
}

float ENGINE::GRAPHICS::DaVinci::Get_Main_Display_Scale()
{
    SDL_DisplayID current_display_id = SDL_GetDisplayForWindow(static_cast<SDL_Window*>(Get_Parent_Window_ptr()));

    if (current_display_id == 0)
    {
        Error("Hey! Get_Main_Display_Scale() + SDL_GetDisplayForWindow() is failing to get ANY display ID");
        return 1.0f;
    }

    return SDL_GetDisplayContentScale(current_display_id);
}


ENGINE::GRAPHICS::DaVinci::E_EventType ENGINE::GRAPHICS::DaVinci::Check_For_Events()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
        case SDL_EVENT_TERMINATING:
            Info("Time to go folks! we got a SDL Quit event");
            return E_EventType::QUIT;


        case SDL_EVENT_WINDOW_RESIZED:
            Resize();
            return E_EventType::WINDOW_RESIZED;


        case SDL_EVENT_WINDOW_HIDDEN:
        case SDL_EVENT_WINDOW_MINIMIZED:
        case SDL_EVENT_WINDOW_OCCLUDED:
            Info("HIDDEN");
            g_Pipeline_Objects.power_mode = 0;
            return E_EventType::WINDOW_IS_HIDDEN;


        case SDL_EVENT_WINDOW_EXPOSED:
        case SDL_EVENT_WINDOW_MAXIMIZED:
        case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
        case SDL_EVENT_WINDOW_MOVED:
        case SDL_EVENT_WINDOW_RESTORED:
            Trace("Not Hidden anymore");
            g_Pipeline_Objects.power_mode = 2;
            return E_EventType::WINDOW_IS_NO_LONGER_HIDDEN;


        case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
            return WINDOW_MOVED_TO_NEW_DISPLAY;


        case SDL_EVENT_LOW_MEMORY:
            WARNc("HEY! LOW MEMORY! BE CAREFUL!")
            g_Pipeline_Objects.power_mode = 1;
            return E_EventType::LOW_MEMORY;


        case SDL_EVENT_WILL_ENTER_BACKGROUND:
        case SDL_EVENT_DID_ENTER_BACKGROUND:
            Trace("Window is no longer main focus");
            g_Pipeline_Objects.power_mode = 1;
            return E_EventType::ENTERED_BACKGROUND;


        case SDL_EVENT_WILL_ENTER_FOREGROUND:
        case SDL_EVENT_DID_ENTER_FOREGROUND:
            Trace("Window is main focus again");
            g_Pipeline_Objects.power_mode = 2;
            return E_EventType::ENTERED_FOREGROUND;


        case SDL_EVENT_SYSTEM_THEME_CHANGED:
            Info("System theme changed");
            return E_EventType::DARK_MODE_THEME_CHANGED;


        case SDL_EVENT_DISPLAY_ADDED:
            Info("New Display was added");
            return E_EventType::DISPLAY_ADDED;


        case SDL_EVENT_DISPLAY_REMOVED:
            Info("Display was removed");
            return E_EventType::DISPLAY_REMOVED;


        case SDL_EVENT_WINDOW_HDR_STATE_CHANGED:
            Info("HDR Status has changed!");
            return E_EventType::DISPLAY_HDR_STATUS_CHANGED;


        // --- Keyboard Events -------------------//
        case SDL_EVENT_KEY_DOWN:
            return E_EventType::KEY_DOWN;
        case SDL_EVENT_KEY_UP:
            return E_EventType::KEY_UP;
        case SDL_EVENT_KEYMAP_CHANGED:
            return E_EventType::KEYMAP_CHANGED;
        // ---------------------------------------//


        // --- Mouse Events ----------------------//
        case SDL_EVENT_WINDOW_MOUSE_ENTER:
            return E_EventType::MOUSE_ENTERED_WINDOW;
        case SDL_EVENT_MOUSE_MOTION:
            return E_EventType::MOUSE_HAS_MOVED;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            return E_EventType::MOUSE_BUTTON_DOWN;
        case SDL_EVENT_MOUSE_BUTTON_UP:
            return E_EventType::MOUSE_BUTTON_UP;
        case SDL_EVENT_MOUSE_WHEEL:
            return E_EventType::MOUSE_WHEEL_MOVED;
        // ---------------------------------------//


        // --- Gamepad Events --------------------//
        case SDL_EVENT_GAMEPAD_AXIS_MOTION:
            return E_EventType::GAMEPAD_AXIS_MOTION;
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
            return E_EventType::GAMEPAD_BUTTON_DOWN;
        case SDL_EVENT_GAMEPAD_BUTTON_UP:
            return E_EventType::GAMEPAD_BUTTON_UP;
        case SDL_EVENT_GAMEPAD_ADDED:
            return E_EventType::GAMEPAD_ADDED;
        case SDL_EVENT_GAMEPAD_REMOVED:
            return E_EventType::GAMEPAD_REMOVED;
        case SDL_EVENT_GAMEPAD_REMAPPED:
            return E_EventType::GAMEPAD_REMAPPED;
        case SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN:
            return E_EventType::GAMEPAD_TOUCHPAD_DOWN;
        case SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION:
            return E_EventType::GAMEPAD_TOUCHPAD_MOTION;
        case SDL_EVENT_GAMEPAD_TOUCHPAD_UP:
            return E_EventType::GAMEPAD_TOUCHPAD_UP;
        // ---------------------------------------//


        // --- Audio Events ----------------------//
        case SDL_EVENT_AUDIO_DEVICE_ADDED:
            return E_EventType::AUDIO_DEVICE_ADDED;
        case SDL_EVENT_AUDIO_DEVICE_REMOVED:
            return E_EventType::AUDIO_DEVICE_REMOVED;
        case SDL_EVENT_AUDIO_DEVICE_FORMAT_CHANGED:
            return E_EventType::AUDIO_DEVICE_FORMAT_CHANGED;
        // ---------------------------------------//
        default:
            break;
        }
    }
    return E_EventType::NOTHING;
}

uint8_t ENGINE::GRAPHICS::DaVinci::Get_Power_Setting() const
{
    return g_Pipeline_Objects.power_mode;
}


bool ENGINE::GRAPHICS::DaVinci::Get_Window_Size(int* width, int* height)
{
    if (SDL_GetWindowSize(static_cast<SDL_Window*>(Parent_Window_ptr), width, height) == false)
    {
        DaVinci::Warn(std::format("Couldn't get the size of SDL_Window because {}", SDL_GetError()));
        return false;
    }
    return true;
}


bool ENGINE::GRAPHICS::DaVinci::Enable_Graphics_Debug_Layer()
{
#if defined(DEBUG) || defined(_DEBUG) || defined(ENGINE_DEBUG_MODE)


    HRESULT debug_created = D3D12GetDebugInterface(IID_PPV_ARGS(&g_Pipeline_Objects.g_DebugInterface));

    if (SUCCEEDED(debug_created))
    {
        g_Pipeline_Objects.g_DebugInterface->EnableDebugLayer();
        return true;
    }
    else
    {
        return false;
    }

#else
    Info("Bro... this ain't debug mode... You cant just enable directx debug layers");
    return false;
#endif
}


Microsoft::WRL::ComPtr<IDXGIAdapter4> ENGINE::GRAPHICS::DaVinci::Get_Best_Graphics_Adapter(bool use_software_renderer)
{
    // Step 1.)  Factory to "make" a graphics card we can use
    ComPtr<IDXGIFactory4> dxgi_Factory;
    UINT createFactoryFlags = 0;

#if defined(DEBUG) || defined(_DEBUG) || defined(ENGINE_DEBUG_MODE)

    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

#endif

    HRESULT factory_success = CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgi_Factory));
    if (FAILED(factory_success))
    {
        Error("Couldn't create DXGIFactory !!! WE DONT KNOW WHAT GRAPHICS CARD TO USE!");
        return nullptr;
    }

    // Step 2.)  Lets find us some GWAPHICS CAWDS!
    ComPtr<IDXGIAdapter1> dxgiAdapter1;
    ComPtr<IDXGIAdapter4> dxgiAdapter4;

    Pipeline_Objects::harness_the_WARP = use_software_renderer;

    if (use_software_renderer == true)
    {
        Info("Trying to make a software renderer");

        HRESULT warp_success = dxgi_Factory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1));
        if (FAILED(warp_success))
        {
            Error("Couldn't create a software renderer card! THE W.A.R.P IS COLLAPSING!");
            return nullptr;
        }

        warp_success = dxgiAdapter1.As(&dxgiAdapter4);
        if (FAILED(warp_success))
        {
            Error("dxgiAdapter4 tomfoolery happening in DaVinci.cpp in Get_Available_Graphics_Adapter()");
        }
    } //end if "use software renderer"

    else
    {
        SIZE_T max_dedicated_video_memory = 0;
        uint8_t potential_adapters_found = 0;

        for (UINT i = 0; dxgi_Factory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
        {
            DXGI_ADAPTER_DESC1 current_adapter_desc;
            dxgiAdapter1->GetDesc1(&current_adapter_desc);

            // Can this card even DO directx 12 ???
            if (
                (current_adapter_desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0
                &&
                SUCCEEDED
                (D3D12CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr))
            )
            {
                potential_adapters_found++;

                // Cool! It's legit! Now is it the BEST one?
                if (current_adapter_desc.DedicatedVideoMemory > max_dedicated_video_memory)
                {
                    max_dedicated_video_memory = current_adapter_desc.DedicatedVideoMemory;
                    HRESULT new_adapter_selected = dxgiAdapter1.As(&dxgiAdapter4);
                    if (FAILED(new_adapter_selected))
                    {
                        Error("HEY! We found a good card but we can't select it... SHIT!");
                    }
                    else
                    {
                        Info(std::format("New Graphics Adapter Selected! It has {} bytes!",
                                         current_adapter_desc.DedicatedVideoMemory));
                        g_Pipeline_Objects.g_GraphicsAdapterDescription = current_adapter_desc;
                    }
                }
            }
        }
    } //end if "use actual GPU"


    return dxgiAdapter4;
}


Microsoft::WRL::ComPtr<ID3D12Device2> ENGINE::GRAPHICS::DaVinci::Create_Device(
    ComPtr<IDXGIAdapter4> which_adapter_to_use)
{
    ComPtr<ID3D12Device2> d3d12Device2;
    HRESULT device_creation_success = D3D12CreateDevice(which_adapter_to_use.Get(), D3D_FEATURE_LEVEL_11_0,
                                                        IID_PPV_ARGS(&d3d12Device2));
    if (FAILED(device_creation_success))
    {
        Error("We couldn't make a DirectX device using the chosen adapter... WHY????");
        return nullptr;
    }

#if defined(DEBUG) || defined(_DEBUG) || defined(ENGINE_DEBUG_MODE)

    ComPtr<ID3D12InfoQueue> pInfoQueue;

    if (SUCCEEDED(d3d12Device2.As(&pInfoQueue)))
    {
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

        // Suppress messages based on their severity level
        D3D12_MESSAGE_SEVERITY Severities[] = {D3D12_MESSAGE_SEVERITY_INFO};

        // Suppress individual messages by their ID
        D3D12_MESSAGE_ID DenyIds[] = {
            D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE, // ??? I have no idea
            D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
            D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
        };

        D3D12_INFO_QUEUE_FILTER NewFilter = {};

        NewFilter.DenyList.NumSeverities = _countof(Severities);
        NewFilter.DenyList.pSeverityList = Severities;
        NewFilter.DenyList.NumIDs = _countof(DenyIds);
        NewFilter.DenyList.pIDList = DenyIds;

        HRESULT debug_setup_success = pInfoQueue->PushStorageFilter(&NewFilter);
        if (FAILED(debug_setup_success))
        {
            Warn("Error in our debugging directx code when setting up debug layer");
        }
    }

#endif

    return d3d12Device2;
}


Microsoft::WRL::ComPtr<ID3D12CommandQueue> ENGINE::GRAPHICS::DaVinci::Create_Command_Queue(ComPtr<ID3D12Device2> device,
    D3D12_COMMAND_LIST_TYPE type)
{
    ComPtr<ID3D12CommandQueue> d3d12CommandQueue;

    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = type; // Copy?    Compute? (Compute can Copy)    Direct? (Does everything)
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL; // Normal, High, Global Realtime
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0; // What GPU do we want to use? GPU #1 or GPU #2 or GPU #0, etc...

    HRESULT command_queue_success = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&d3d12CommandQueue));
    if (FAILED(command_queue_success))
    {
        Error("WE MESSED UP MAKING THE COMMAND QUEUE! HOW?");
        return nullptr;
    }

    return d3d12CommandQueue;
}


bool ENGINE::GRAPHICS::DaVinci::Check_Variable_Refresh_Rate_Support()
{
    BOOL allowTearing = FALSE;

    // Thanks https://www.3dgep.com/learning-directx-12-1/
    // Rather than create the DXGI 1.5 factory interface directly, we create the
    // DXGI 1.4 interface and query for the 1.5 interface. This is to enable the 
    // graphics debugging tools which will not support the 1.5 factory interface 
    // until a future update.
    ComPtr<IDXGIFactory4> factory4;
    if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
    {
        ComPtr<IDXGIFactory5> factory5;
        if (SUCCEEDED(factory4.As(&factory5)))
        {
            if (FAILED(factory5->CheckFeatureSupport(
                DXGI_FEATURE_PRESENT_ALLOW_TEARING,
                &allowTearing, sizeof(allowTearing))))
            {
                allowTearing = FALSE;
            }
        }
    }

    Render_Settings::Display_Settings::g_VariableRefreshRate = allowTearing == TRUE;

    return allowTearing == TRUE;
}


Microsoft::WRL::ComPtr<IDXGISwapChain4> ENGINE::GRAPHICS::DaVinci::Create_Swap_Chain(
    HWND hWnd, ComPtr<ID3D12CommandQueue> commandQueue, uint32_t width, uint32_t height)
{
    ComPtr<IDXGISwapChain4> dxgiSwapChain4;
    ComPtr<IDXGIFactory4> dxgiFactory4;

    UINT createFactoryFlags = 0;

#if defined(DEBUG) || defined(_DEBUG) || defined(ENGINE_DEBUG_MODE)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    if (FAILED(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4))))
    {
        Error("HEY! WE couldn't make a swapchain! Specifically the DXGI factory! CreateDXGIFactory2()");
        Error("Failure in Create_Swap_Chain()");
        return nullptr;
    }

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = g_Pipeline_Objects.g_frameBufferFormat;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc = {1, 0};
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // Is this for the screen, or for a shader input?
    swapChainDesc.BufferCount = g_Pipeline_Objects.frames_in_flight;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH; // We can stretch, ratio-stretch, or do nothing
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // Sequential Flip model is better for 2D UI's
    swapChainDesc.AlphaMode = g_Pipeline_Objects.g_frameAlphaBehaviour;
    swapChainDesc.Flags = Check_Variable_Refresh_Rate_Support() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

    ComPtr<IDXGISwapChain1> swapChain1;

    if (FAILED(dxgiFactory4->CreateSwapChainForHwnd(
        commandQueue.Get(), hWnd, &swapChainDesc, nullptr, nullptr, &swapChain1)))
    {
        Error("HEY! WE couldn't make a swapchain! Specifically the Create Swap chain! CreateSwapChainForHwnd()");
        Trace(std::format("dxgiFactory4 == nullptr: {}", (dxgiFactory4 == nullptr || dxgiFactory4.Get() == nullptr)));
        Trace(std::format("commandQueue == nullptr: {}", (commandQueue == nullptr || commandQueue.Get() == nullptr)));
        Trace(std::format("hWnd == NULL: {} - Name: {}", hWnd == NULL, Get_Window_Name_From_HWND(hWnd)));
        Trace(std::format("swapChainDesc == nullptr: {}", &swapChainDesc == nullptr));
        Trace(std::format("swapChain1 == nullptr: {}", (swapChain1 == nullptr || swapChain1.Get() == nullptr)));
        Error("Failure in Create_Swap_Chain()");

        return nullptr;
    }

    // Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
    // will be handled manually.
    if (FAILED((dxgiFactory4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER))))
    {
        Warn("Oh CRAP! Alt+Enter will continue to toggle between fullscreen-windowed");
        Warn("MakeWindowAssociation() failed");
    }

    if (FAILED(swapChain1.As(&dxgiSwapChain4)))
    {
        Error(
            "Error with either our IDXGISwapChain1, or our IDXGISwapChain4... check out Create_Swap_Chain() in Davinci.cpp");
        return nullptr;
    }

    return dxgiSwapChain4;
}


Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> ENGINE::GRAPHICS::DaVinci::Create_Descriptor_Heap(
    ComPtr<ID3D12Device2> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors)
{
    ComPtr<ID3D12DescriptorHeap> descriptorHeap;

    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = numDescriptors;
    desc.Type = type;

    if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap))))
    {
        Error("HEY! WE couldn't create your descriptor heap! investigate Create_Descriptor_Heap in Davinci.cpp");
        return nullptr;
    }

    return descriptorHeap;
}


bool ENGINE::GRAPHICS::DaVinci::Update_Render_Target_Views(ComPtr<ID3D12Device2> device,
                                                           ComPtr<IDXGISwapChain4> swapChain,
                                                           ComPtr<ID3D12DescriptorHeap> descriptorHeap)
{
    // Can't say UINT because compiler complains
    auto render_target_view_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    CD3DX12_CPU_DESCRIPTOR_HANDLE render_target_view_handle(descriptorHeap->GetCPUDescriptorHandleForHeapStart());

    for (UINT i = 0; i < g_Pipeline_Objects.frames_in_flight; ++i)
    {
        ComPtr<ID3D12Resource> back_buffer;

        if (FAILED(swapChain->GetBuffer(i, IID_PPV_ARGS(&back_buffer))))
        {
            Error("Our Swap-Chain couldn't get the back buffer! Where are we rendering our screen to??");
            return false;
        }

        device->CreateRenderTargetView(back_buffer.Get(), nullptr, render_target_view_handle);

        g_Pipeline_Objects.g_BackBuffers[i] = back_buffer;

        render_target_view_handle.Offset(render_target_view_descriptor_size);
    }

    return true;
}


Microsoft::WRL::ComPtr<ID3D12CommandAllocator> ENGINE::GRAPHICS::DaVinci::Create_Command_Allocator(
    ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type)
{
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    if (FAILED(device->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator) )))
    {
        Error("HEY! We couldn't make a Command Allocator! investigate CreateCommandAllocator() in DaVinci.cpp");
        return nullptr;
    }

    return commandAllocator;
}


Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> ENGINE::GRAPHICS::DaVinci::Create_Command_List(
    ComPtr<ID3D12Device2> device,
    ComPtr<ID3D12CommandAllocator> commandAllocator,
    D3D12_COMMAND_LIST_TYPE type)
{
    ComPtr<ID3D12GraphicsCommandList> commandList;
    if (FAILED(device->CreateCommandList(0, type, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList))))
    {
        Error(std::format("Command List couldn't be made! Was the command Allocator null? : {}",
                          commandAllocator == nullptr));
        return nullptr;
    }

    if (FAILED(commandList->Close()))
    {
        Error(std::format(
            "The Command list was created successfully... but now it can't be closed? WHAT? The Command list is null? : {}",
            commandList == nullptr));
        return nullptr;
    }

    return commandList;
}


Microsoft::WRL::ComPtr<ID3D12Fence> ENGINE::GRAPHICS::DaVinci::Create_Fence(ComPtr<ID3D12Device2> device)
{
    ComPtr<ID3D12Fence> fence;

    if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))))
    {
        Error("WE COULDN'T MAKE A FENCE! Everything on the GPU and CPU is outta wack!");
        return nullptr;
    }

    return fence;
}


HANDLE ENGINE::GRAPHICS::DaVinci::Create_Event_Handle()
{
    HANDLE fenceEvent;
    fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

    assert(fenceEvent && "Failed to create fence event.");

    return fenceEvent;
}


uint64_t ENGINE::GRAPHICS::DaVinci::Signal(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence,
                                           uint64_t& fenceValue)
{
    uint64_t fenceValueForSignal = ++fenceValue;

    if (FAILED(commandQueue->Signal(fence.Get(), fenceValueForSignal)))
    {
        Error("We can't signal our new fence value!!! why??");
    }

    return fenceValueForSignal;
}


void ENGINE::GRAPHICS::DaVinci::Wait_For_Fence_Value(ComPtr<ID3D12Fence> fence, uint64_t fenceValue, HANDLE fenceEvent,
                                                     std::chrono::milliseconds duration)
{
    if (fence->GetCompletedValue() < fenceValue)
    {
        if (FAILED(fence->SetEventOnCompletion(fenceValue, fenceEvent)))
        {
            Error("We couldn't successfully wait on our Fence? Investigate Wait_For_Fence_Value()");
        }
        ::WaitForSingleObject(fenceEvent, static_cast<DWORD>(duration.count()));
    }
}


void ENGINE::GRAPHICS::DaVinci::Flush(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence,
                                      uint64_t& fenceValue, HANDLE fenceEvent)
{
    uint64_t fenceValueForSignal = Signal(commandQueue, fence, fenceValue);
    
    Wait_For_Fence_Value(fence, fenceValueForSignal, fenceEvent);
}


void ENGINE::GRAPHICS::DaVinci::Update()
{
    static uint64_t frameCounter = 0;
    static double elapsedSeconds = 0.0;


    frameCounter++;

    if (Pipeline_Objects::is_first_frame)
    {
        return;
    }


    auto deltaTime = end_frame_render_time - start_frame_render_time;

    start_frame_render_time = end_frame_render_time;

    elapsedSeconds += deltaTime.count() * 1e-9;

    if (elapsedSeconds > 1.0)
    {
        auto fps = frameCounter / elapsedSeconds;
        Trace(std::format("FPS: {}", fps));
        frameCounter = 0;
        elapsedSeconds = 0.0;
    }
    start_frame_render_time = std::chrono::high_resolution_clock::now();
}


void ENGINE::GRAPHICS::DaVinci::Render()
{
    // Before any commands can be recorded into the command list, 
    // the command allocator and command list needs to be reset to its initial state
    auto commandAllocator = g_Pipeline_Objects.g_CommandAllocators[g_Pipeline_Objects.g_CurrentBackBufferIndex];

    auto backBuffer = g_Pipeline_Objects.g_BackBuffers[g_Pipeline_Objects.g_CurrentBackBufferIndex];

    commandAllocator->Reset();

    g_Pipeline_Objects.g_CommandList->Reset(commandAllocator.Get(), nullptr);


    // Before the render target can be cleared, it must be transitioned to the RENDER_TARGET state.

    // Clear the render target.
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            backBuffer.Get(),
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET);

        g_Pipeline_Objects.g_CommandList->ResourceBarrier(1, &barrier);


        CD3DX12_CPU_DESCRIPTOR_HANDLE render_target_view(
            g_Pipeline_Objects.g_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
            g_Pipeline_Objects.g_CurrentBackBufferIndex,
            g_Pipeline_Objects.g_RTVDescriptorSize);


        g_Pipeline_Objects.g_CommandList->ClearRenderTargetView(
            render_target_view,
            g_Pipeline_Objects.clear_colour,
            0,
            nullptr);
    }


    // Present time!
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            backBuffer.Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT);

        g_Pipeline_Objects.g_CommandList->ResourceBarrier(1, &barrier);

        if (FAILED(g_Pipeline_Objects.g_CommandList->Close()))
        {
            Error("We couldn't close the command list in Render() in Da Vinci.cpp");
            return;
        }

        ID3D12CommandList* const commandLists[] =
        {
            g_Pipeline_Objects.g_CommandList.Get()

        };

        g_Pipeline_Objects.g_CommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

        UINT syncInterval = Render_Settings::Display_Settings::g_VSync ? 1 : 0;

        // power mode == 1: Render but slower... not in focus  -  power mode == 2: Render as normal
        syncInterval = g_Pipeline_Objects.power_mode == 1 ? (syncInterval + 1) : syncInterval;

        UINT presentFlags = Render_Settings::Display_Settings::g_VariableRefreshRate
                            && !Render_Settings::Display_Settings::g_VSync
                                ? DXGI_PRESENT_ALLOW_TEARING
                                : 0;

        if (FAILED(g_Pipeline_Objects.g_SwapChain->Present(syncInterval, presentFlags)))
        {
            Error("HOW THE HELL DID OUR SWAPCHAIN PRESENT FAIL? Investigate Render() and the Present()");
        }

        g_Pipeline_Objects.g_FrameFenceValues[g_Pipeline_Objects.g_CurrentBackBufferIndex]
            = Signal(g_Pipeline_Objects.g_CommandQueue, g_Pipeline_Objects.g_Fence, g_Pipeline_Objects.g_FenceValue);

        g_Pipeline_Objects.g_CurrentBackBufferIndex = g_Pipeline_Objects.g_SwapChain->GetCurrentBackBufferIndex();

        Wait_For_Fence_Value(g_Pipeline_Objects.g_Fence,
                             g_Pipeline_Objects.g_FrameFenceValues[g_Pipeline_Objects.g_CurrentBackBufferIndex],
                             g_Pipeline_Objects.g_FenceEvent);
    }
}

void ENGINE::GRAPHICS::DaVinci::Resize()
{
    int current_width, current_height;

    Get_Window_Size(&current_width, &current_height);

    if (current_width != g_Pipeline_Objects.g_FrameBufferWidth
        || current_height != g_Pipeline_Objects.g_FrameBufferWidth)
    {
        // Don't allow 0 size swap chain back buffers.
        g_Pipeline_Objects.g_FrameBufferWidth = std::max(1, current_width);
        g_Pipeline_Objects.g_FrameBufferWidth = std::max(1, current_height);

        // Flush the GPU queue to make sure the swap chain's back buffers
        // are not being referenced by an in-flight command list.
        Flush(g_Pipeline_Objects.g_CommandQueue,
              g_Pipeline_Objects.g_Fence,
              g_Pipeline_Objects.g_FenceValue,
              g_Pipeline_Objects.g_FenceEvent);


        for (int i = 0; i < g_Pipeline_Objects.frames_in_flight; ++i)
        {
            // Any references to the back buffers must be released
            // before the swap chain can be resized.
            g_Pipeline_Objects.g_BackBuffers[i].Reset();

            g_Pipeline_Objects.g_FrameFenceValues[i]
                = g_Pipeline_Objects.g_FrameFenceValues[g_Pipeline_Objects.g_CurrentBackBufferIndex];
        }

        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

        if (FAILED(g_Pipeline_Objects.g_SwapChain->GetDesc(&swapChainDesc)))
        {
            Error("We Failed to resize the window because we couldn't get the swapchain desc {} in Resize()");
        }

        if (FAILED(g_Pipeline_Objects.g_SwapChain->ResizeBuffers(
            g_Pipeline_Objects.frames_in_flight,
            g_Pipeline_Objects.g_FrameBufferWidth,
            g_Pipeline_Objects.g_FrameBufferHeight,
            swapChainDesc.BufferDesc.Format,
            swapChainDesc.Flags)))
        {
            Error("We Failed to resize the window because we couldn't make the swapchain ResizeBuffers() in Resize()");
        }

        g_Pipeline_Objects.g_CurrentBackBufferIndex = g_Pipeline_Objects.g_SwapChain->GetCurrentBackBufferIndex();
        Update_Render_Target_Views(g_Pipeline_Objects.g_Device,
                                   g_Pipeline_Objects.g_SwapChain,
                                   g_Pipeline_Objects.g_RTVDescriptorHeap);
    }

    Info(std::format("Window Resized to {}px, {}px", current_width, current_height));
}


void ENGINE::GRAPHICS::DaVinci::Set_FullScreen(bool use_fullscreen)
{
    if (Render_Settings::Display_Settings::g_Fullscreen != use_fullscreen)
    {
        Render_Settings::Display_Settings::g_Fullscreen = use_fullscreen;

        SDL_SetWindowFullscreen(static_cast<SDL_Window*>(Get_Parent_Window_ptr()),
                                Render_Settings::Display_Settings::g_Fullscreen);
    }
}


void ENGINE::GRAPHICS::DaVinci::Finish_Frame()
{
    g_Pipeline_Objects.is_first_frame = false;
    end_frame_render_time = std::chrono::high_resolution_clock::now();
}


std::string ENGINE::GRAPHICS::DaVinci::Get_Window_Name_From_HWND(HWND hWnd)
{
    char buffer[512];
    GetWindowTextA(hWnd, buffer, 255);
    buffer[511] = '\0';
    std::string result = buffer;

    return result;
}


void ENGINE::GRAPHICS::DaVinci::Set_Clear_Colour(float red, float green, float blue, float alpha)
{
    g_Pipeline_Objects.clear_colour[0] = red;
    g_Pipeline_Objects.clear_colour[1] = green;
    g_Pipeline_Objects.clear_colour[2] = blue;
    g_Pipeline_Objects.clear_colour[3] = alpha;
}


ComPtr<ID3D12Device2> ENGINE::GRAPHICS::DaVinci::Get_Active_Device() const
{
    return g_Pipeline_Objects.g_Device;
}


ComPtr<ID3D12CommandQueue> ENGINE::GRAPHICS::DaVinci::Get_Active_CommandQueue() const
{
    return g_Pipeline_Objects.g_CommandQueue;
}


int ENGINE::GRAPHICS::DaVinci::Get_Number_of_Frames_In_Flight() const
{
    return g_Pipeline_Objects.frames_in_flight;
}


DXGI_FORMAT ENGINE::GRAPHICS::DaVinci::Get_RTV_Frame_Buffer_Format() const
{
    return g_Pipeline_Objects.g_frameBufferFormat;
}


bool ENGINE::GRAPHICS::DaVinci::Update_Buffer_Resource(ComPtr<ID3D12GraphicsCommandList2> commandList,
                                                       ID3D12Resource** pDestinationResource,
                                                       ID3D12Resource** pIntermediateResource,
                                                       size_t numElements,
                                                       size_t elementSize,
                                                       const void* bufferData,
                                                       D3D12_RESOURCE_FLAGS flags)
{
    size_t bufferSize = numElements * elementSize;
    // Create a committed resource for the GPU resource in a default heap.
    if (FAILED(g_Pipeline_Objects.g_Device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags),
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(pDestinationResource))))
    {
        Error("Hey! Update_Buffer_Resource() failed to create a GPU resource in committed memory");
        return false;
    }

    // Create a committed resource for the upload.

    // TODO: It's 1am as I write this... I am delirious and this must be re-learnt
    if (bufferData)
    {
        if (FAILED(g_Pipeline_Objects.g_Device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(pIntermediateResource))))
        {
            Error(
                "We MADE the resource, but we cant upload it... shit, CreateCommittedResource() in Update_Buffer_Resource(0 is to blame");

            D3D12_SUBRESOURCE_DATA subresourceData = {};
            subresourceData.pData = bufferData;
            subresourceData.RowPitch = bufferSize;
            subresourceData.SlicePitch = subresourceData.RowPitch;

            UpdateSubresources(commandList.Get(),
                               *pDestinationResource,
                               *pIntermediateResource,
                               0,
                               0,
                               1,
                               &subresourceData);
        }
    }
}


void ENGINE::GRAPHICS::DaVinci::transition_resource(ComPtr<ID3D12GraphicsCommandList2> commandList,
                                                    ComPtr<ID3D12Resource> resource,
                                                    D3D12_RESOURCE_STATES beforeState,
                                                    D3D12_RESOURCE_STATES afterState)
{
}

void ENGINE::GRAPHICS::DaVinci::Resize_Depth_Buffer(int width, int height)
{

}




{
    if (m_ContentLoaded)
    {
        // Flush any GPU commands that might be referencing the depth buffer.
        Application::Get().Flush();

        width = std::max(1, width);
        height = std::max(1, height);

        auto device = Application::Get().GetDevice();

        // Resize screen dependent resources.
        // Create a depth buffer.
        D3D12_CLEAR_VALUE optimizedClearValue = {};
        optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        optimizedClearValue.DepthStencil = { 1.0f, 0 };

        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height,
                1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &optimizedClearValue,
            IID_PPV_ARGS(&m_DepthBuffer)
        ));

        // Update the depth-stencil view.
        D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
        dsv.Format = DXGI_FORMAT_D32_FLOAT;
        dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsv.Texture2D.MipSlice = 0;
        dsv.Flags = D3D12_DSV_FLAG_NONE;

        device->CreateDepthStencilView(m_DepthBuffer.Get(), &dsv,
            m_DSVHeap->GetCPUDescriptorHandleForHeapStart());
    }
}