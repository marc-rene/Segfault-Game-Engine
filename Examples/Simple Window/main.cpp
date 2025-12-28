#include <iostream>
#include <chrono>
#include <thread>

#include "Apocalypse.hpp"



int main()
{
    ENGINE::Runtime::ClientRuntime clientRuntime;
    std::string NewTitle;

    clientRuntime.On_First_Ever_Frame();

    uMint counter = 0;

    //clientRuntime.EnableEditor();

    std::thread FixTickThread(&ENGINE::Runtime::ClientRuntime::On_FixedTick, &clientRuntime);
    
    if (FixTickThread.joinable())
    {
        INFOc("Trying to make the Fix Tick Thread Detach!");
        FixTickThread.detach();
    }

    while (clientRuntime.IsRunning())
    {
        auto startTime = std::chrono::high_resolution_clock::now();
        
        clientRuntime.On_Frame_Start();
        

        //ENGINE::Editor::Windows::FPS_Counter();

        clientRuntime.On_Frame_End();
        

        if (counter == 0)
        {
            NewTitle = std::format("FPS: {}    {} us between frames", 1000000.0f / clientRuntime.GetFixedTickDeltaTimeMicroSeconds_Average(), clientRuntime.GetFixedTickDeltaTimeMicroSeconds_Average());
            ENGINE::Platform::WindowManager::GetInstance()->SetWindowTitle(NewTitle, true);
        }
        counter++;
    }
    FixTickThread.join();

    clientRuntime.Shutdown(); // Just incase something else changes clientRuntime.IsRunning()
    return 0;
}