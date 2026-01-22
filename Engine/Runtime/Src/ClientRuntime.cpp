#include "../Include/ClientRuntime.hpp"

#include "DaVinci.hpp"


ENGINE::RUNTIME::ClientRuntime::ClientRuntime() : ENGINE_MODULE_INTERFACE("Client Runtime")
{
    DaVinci_Instance = ENGINE::GRAPHICS::DaVinci();
    DaVinci_Instance.Initialise_Context(true);
    KeepRunning = true;
}

bool ENGINE::RUNTIME::ClientRuntime::Create_Window(std::string Parent_Window_Name, int Initial_Width, int Initial_Height)
{
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


