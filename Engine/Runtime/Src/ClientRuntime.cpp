#include "../Include/ClientRuntime.hpp"

#include "DaVinci.hpp"
#include "ECS/Src/entt.hpp"


ENGINE::RUNTIME::ClientRuntime::ClientRuntime() : ENGINE_MODULE_INTERFACE("Client Runtime")
{
    DaVinci_Instance = GRAPHICS::DaVinci();
    GRAPHICS::DaVinci::Set_New_DaVinci_Instance(&DaVinci_Instance);

    DaVinci_Instance.Initialise_Context(true);
    KeepRunning = true;
}


bool ENGINE::RUNTIME::ClientRuntime::Enable_Editor()
{
    // Is DaVinci even legit?
    if (DaVinci_Instance.Get_Parent_Window_ptr() == nullptr)
    {
        Error("Hey! We cant initialise the editor... there's no initialised DaVinci for it to hop into");
        return false;
    }
    
    Nitpick_Editor_Instance = EDITOR::Nitpick::Nitpick();
    EDITOR::Nitpick::Set_New_Editor_Instance(&Nitpick_Editor_Instance);
    
    if (Nitpick_Editor_Instance.Set_New_Host_DaVinci_Instance(&DaVinci_Instance) == false)
    {
        Error("Our Davinci reference is good apparently... but our editor ain't having it!??");
        Nitpick_Editor_Instance;
        return false;
    }
    
    return true;
}

bool ENGINE::RUNTIME::ClientRuntime::Get_Window_Size(int* p_width, int* p_height)
{
    if (DaVinci_Instance.Get_Window_Size(p_width, p_height) == false)
    {
        Warn("Something's wrong with DaVinci... gonna say the window width is 600 but dude it's messed up");
        *p_width = 800;
        *p_height = 600;
        return false;
    }
    return true;
}


int ENGINE::RUNTIME::ClientRuntime::Get_Window_Height()
{
    int height;
    Get_Window_Size(&height, NULL);
    return height;
}

int ENGINE::RUNTIME::ClientRuntime::Get_Window_Width()
{
    int width;
    Get_Window_Size(NULL, &width);
    return width;
}


bool ENGINE::RUNTIME::ClientRuntime::Create_Window(std::string Parent_Window_Name, int Initial_Width,
                                                   int Initial_Height)
{
    // Check for DirectX Math library support.
    if (!DirectX::XMVerifyCPUSupport())
    {
        Critical("Failed to verify DirectX Math library support");
    }
    return DaVinci_Instance.New_Parent_Window(Parent_Window_Name, Initial_Width, Initial_Height);
}

void ENGINE::RUNTIME::ClientRuntime::Pre_tick()
{
    if (DaVinci_Instance.Check_For_Events() == GRAPHICS::DaVinci::E_EventType::QUIT)
    {
        Info("DaVinci is quitting...");
        Shutdown_Mutex.lock();
        KeepRunning = false;
        Shutdown_Mutex.unlock();
        Info("'KeepRunning' is now false");
    }
}

void ENGINE::RUNTIME::ClientRuntime::Tick()
{
    if (DaVinci_Instance.Get_Power_Setting() > 0)
    {
        DaVinci_Instance.Update();
        DaVinci_Instance.Render();
    }
}

void ENGINE::RUNTIME::ClientRuntime::Post_tick()
{
    DaVinci_Instance.Finish_Frame();
}


ENGINE::GRAPHICS::DaVinci* ENGINE::RUNTIME::ClientRuntime::Get_DaVinci_instance()
{
    return &DaVinci_Instance;
}
