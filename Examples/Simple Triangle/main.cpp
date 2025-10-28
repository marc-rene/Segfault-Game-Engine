#include <iostream>

#include "Apocalypse.hpp"



int main()
{
    ENGINE::Runtime::ClientRuntime clientRuntime;


    while (clientRuntime.IsRunning())
    {
        clientRuntime.On_FixedTick_Start();

        clientRuntime.On_FixedTick_End();
    }

    clientRuntime.Shutdown(); // Just incase something else changes clientRuntime.IsRunning()
    return 0;
}