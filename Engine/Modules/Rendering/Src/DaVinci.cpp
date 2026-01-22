#include "../Include/DaVinci.hpp"

#include <d3dx12.h>

#include <SDL3/SDL_init.h>
#include "SDL3/SDL_video.h"


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
    return parent_window_hwnd;
}


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


bool ENGINE::GRAPHICS::DaVinci::Initialise_Context(bool use_default)
{
    SDL_InitFlags init_flags = SDL_INIT_VIDEO;

    if (!use_default)
    {
        Error("Look, I know you don't want to use config vars, but we got a TODO for that... please wait");
    }

    return SDL_Init(SDL_INIT_VIDEO);
}


ENGINE::GRAPHICS::DaVinci::E_EventType ENGINE::GRAPHICS::DaVinci::Check_For_Events()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:

            DaVinci::Info("Time to go folks! we got a SDL Quit event");

            return E_EventType::QUIT;
            break;
        default:
            break;
        }
    }
    return E_EventType::NOTHING;
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


bool ENGINE::GRAPHICS::DaVinci::EnableGraphicsDebugLayer()
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
    Info("Bro... this ain't debug mode... You cant just enable directx debug layers")
    return false;
#endif
}


Microsoft::WRL::ComPtr<IDXGIAdapter4> ENGINE::GRAPHICS::DaVinci::Get_Best_Graphics_Adapter()
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

    if (Pipeline_Objects::harness_the_WARP == true)
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

    return allowTearing == TRUE;
}



Microsoft::WRL::ComPtr<IDXGISwapChain4> ENGINE::GRAPHICS::DaVinci::Create_Swap_Chain(HWND hWnd,
    ComPtr<ID3D12CommandQueue> commandQueue)
{
    // TODO: Finish this tomorrow
    return nullptr;
}
