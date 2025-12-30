#include "Editor.hpp"

void ENGINE::Editor::OnFrameStart()
{
    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    try
    {
        ImGui::NewFrame();
    }
    catch (std::exception& e)
    {
        WARN(ENGINE::Editor::TITLE, "There was an Editor issue: {}", e.what());
        if (ImGui::GetIO().BackendRendererUserData == nullptr)
        {
            ImGui_ImplDX11_Init(
                ENGINE::Rendering::RenderMaster::GetInstance()->device_ptr,
                ENGINE::Rendering::RenderMaster::GetInstance()->device_context_ptr);
        }
    }
}


void ENGINE::Editor::ComputeDrawData()
{
    ImGui::Render();
}

void ENGINE::Editor::RenderDrawData()
{
    // Rendering
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}


ENGINE::Editor::Editor()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup scaling
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForD3D(ENGINE::Platform::WindowManager::GetInstance()->GetMainWindowRef());
    ImGui_ImplDX11_Init(
        ENGINE::Rendering::RenderMaster::GetInstance()->device_ptr,
        ENGINE::Rendering::RenderMaster::GetInstance()->device_context_ptr);

    b_initialised = true;
}