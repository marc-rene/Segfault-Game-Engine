#include <iostream>

#include "Apocalypse.hpp"



int main()
{
    //ENGINE::Log::SetLoggerVerbosity(3); // Warnings Only

    ENGINE::Runtime::ClientRuntime clientRuntime;


    while (clientRuntime.IsRunning())
    {
        clientRuntime.On_FixedTick_Start();

        clientRuntime.On_FixedTick_End();

        // std::cout << "Fixed Tick Deltatime: " << clientRuntime.GetFixedTickDeltaTimeMicroSeconds() << " us\t==\t" << std::endl;
    }

    // clientRuntime.Shutdown(); // All cleanup handled by deconstructors
    return 0;
}