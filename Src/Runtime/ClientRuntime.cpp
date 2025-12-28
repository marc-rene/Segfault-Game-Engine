#include "ClientRuntime.hpp"


void ENGINE::Runtime::ClientRuntime::On_First_Ever_Frame()
{
    if (ENGINE::Rendering::RenderMaster::GetInstance()->Initialise() == false) {
        WARN("ENGINE::Runtime", "Trying to initialise our RenderMaster Instance was a failure!");
    }
    else {
        INFO("ENGINE::Runtime", "Trying to initialise our RenderMaster Instance was a Great Success!");
    }
}



void ENGINE::Runtime::ClientRuntime::On_Frame_Start()
{
    ENGINE::Rendering::RenderMaster::GetInstance()->On_RenderStart();

    if (ENGINE::Editor::Is_Initialised())
        ENGINE::Editor::GetInstance()->OnFrameStart();
}

void ENGINE::Runtime::ClientRuntime::On_Frame_End()
{
    ENGINE::Rendering::RenderMaster::GetInstance()->On_RenderEnd();
}



void ENGINE::Runtime::ClientRuntime::On_FixedTick_Start()
{
    DT_Mutex.lock();
    // START THE CLOCK 
    FixedTickStartTimestamp = std::chrono::high_resolution_clock::now();
    DT_Mutex.unlock();

    EventPolling_Mutex.lock();
    switch (ENGINE::Platform::WindowManager::PollEvents())
    {
    case 0:
        Shutdown();
        break;

    case 1:
        ENGINE::Rendering::RenderMaster::GetInstance()->ResizeSwapChainBuffer();
        break;
    }
    EventPolling_Mutex.unlock();
}


void ENGINE::Runtime::ClientRuntime::On_FixedTick()
{
    while (IsRunning())
    {
        On_FixedTick_Start();
        On_FixedTick_Body();
        On_FixedTick_End();

        static const double total_time_between_ticks_us = (1.0 / FixedTickRate) * 1e+6; // Go from 0.0166667 seconds to microseconds
        static long long us_wait_time;

        us_wait_time = total_time_between_ticks_us - LastFixedTickDeltaTimeMicroSeconds.count();
        //INFO("ENGINE::Runtime::ClientRuntime", "Tick! Waiting for {} us so that we can stay to our 1/{} tick rate... Thread ID: {}", us_wait_time, (int)(FixedTickRate), std::this_thread::get_id());
        INFO("ENGINE::Runtime::ClientRuntime", "Tick! Wait for {} us", us_wait_time);

        std::this_thread::sleep_for(std::chrono::microseconds(us_wait_time));
    }
};

void ENGINE::Runtime::ClientRuntime::On_FixedTick_End()
{
    DT_Mutex.lock();
    // STOP THE CLOCK 
    auto FinishedTimestamp = std::chrono::high_resolution_clock::now();

    LastFixedTickDeltaTimeMicroSeconds = std::chrono::duration_cast<std::chrono::microseconds>(FinishedTimestamp - FixedTickStartTimestamp);

    if (LastFixedTickDeltaTimeMicroSeconds.count() < 1) // We cant have 0 microseconds
        LastFixedTickDeltaTimeMicroSeconds.zero();

    AverageFixedTickDT_DataSet[FenceValue % 32] = LastFixedTickDeltaTimeMicroSeconds.count();

    FenceValue++;
    DT_Mutex.unlock();
}