#include "../Include/Nitpick.hpp"


#include "imgui.h"

#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_dx12.h"


ENGINE::EDITOR::Nitpick::Nitpick() : ENGINE_MODULE_INTERFACE("Nit-Pick Editor")
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
}


void ENGINE::EDITOR::Nitpick::Set_New_Editor_Instance(Nitpick* new_instance_ptr)
{
    self_ptr = new_instance_ptr;
}

bool ENGINE::EDITOR::Nitpick::Set_New_Host_DaVinci_Instance(GRAPHICS::DaVinci* new_host_davinci)
{
    if (new_host_davinci == nullptr || new_host_davinci->Get_Parent_Window_ptr() == nullptr)
    {
        Error("The Davinci instance we were given is invalid");
        return false;
    }

    Info("Hooked up to a new DaVinci Host!");


    // Setup scaling
    float main_scale = new_host_davinci->Get_Main_Display_Scale();
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;
    Info("Got Scaling off DaVinci Host!");

    ImGui_ImplSDL3_InitForD3D(static_cast<SDL_Window*>(new_host_davinci->Get_Parent_Window_ptr()));
    Info("SDL init for directx 12 was a success!");

    ImGui_ImplDX12_InitInfo init_info = {};
    init_info.Device = new_host_davinci->Get_Active_Device().Get();
    init_info.CommandQueue = new_host_davinci->Get_Command_Queue().Get();
    init_info.NumFramesInFlight = new_host_davinci->Get_Number_of_Frames_In_Flight();
    init_info.RTVFormat = new_host_davinci->Get_RTV_Frame_Buffer_Format();
    init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;

    // ImGui has an assert that CAN'T be caught
    try
    {
        if (ImGui_ImplDX12_Init(&init_info) == false)
        {
            throw std::runtime_error("Failed to initialise ImGui with current directx 12 context!");
        }
    }
    catch (std::exception& e)
    {
        Error("Failed to initialize the Nitpick Editor with our current Davinci host... maybe the SRV wasn't setup yet?");
        Error(e.what());
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
        return false;
        
    }

    return true;
}
