#pragma once

#include <chrono>
#include <mutex>

#include "DaVinci.hpp"
#include "TypeDefinitions.hpp"
#include "Log.hpp"


namespace ENGINE::RUNTIME
{
    class ClientRuntime
    {
    public:
        ClientRuntime();

        bool CreateWindow(std::string Parent_Window_Name, int Initial_Width, int Initial_Height);

        void Pre_tick();
        void Tick();
        void Post_tick();
        
        bool* Is_Running()
        {
            return &KeepRunning;
        }

    private:
        ENGINE::GRAPHICS::DaVinci DaVinci_Instance;
        inline static bool KeepRunning = false;
        std::mutex Shutdown_Mutex;

        const uMint FixedTickRate = 60;
        inline static unsigned long long FenceValue = 0;
        inline static std::chrono::steady_clock::time_point FixedTickStartTimestamp;
        inline static std::chrono::microseconds LastFixedTickDeltaTimeMicroSeconds;
        inline static long long AverageFixedTickDT_DataSet[32] = {};
        inline static std::mutex DT_Mutex;
        inline static std::mutex EventPolling_Mutex;
    };
};
