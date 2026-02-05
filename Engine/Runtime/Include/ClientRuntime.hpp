#pragma once

#include <chrono>
#include <mutex>

#include "DaVinci.hpp"
#include "Nitpick.hpp"
#include "Definitions_Global.hpp"
#include "Log.hpp"


namespace ENGINE::RUNTIME
{
    class ClientRuntime : ENGINE_MODULE_INTERFACE
    {
    public:
        ClientRuntime();

        
        // Do we want to load a custom config from file? or use Default?
        bool Load_Config_Variables();
        
        
        // Should be called AFTER we've made a window
        bool Enable_Editor();
        
        bool Get_Window_Size(int*, int*);
        int Get_Window_Width();
        int Get_Window_Height();
        
         
        
        bool Create_Window(std::string Parent_Window_Name, int Initial_Width, int Initial_Height);


        void Pre_tick();
        void Tick();
        void Post_tick();

                
        GRAPHICS::DaVinci* Get_DaVinci_instance();


        bool* Is_Running()
        {
            return &KeepRunning;
        }

    private:
        ENGINE::GRAPHICS::DaVinci DaVinci_Instance;
        
        ENGINE::EDITOR::Nitpick Nitpick_Editor_Instance;
        inline static bool KeepRunning = false;
        std::mutex Shutdown_Mutex;

        const uMint FixedTickRate = 60;
        inline static std::chrono::steady_clock::time_point FixedTickStartTimestamp;
        inline static std::chrono::microseconds LastFixedTickDeltaTimeMicroSeconds;
        inline static long long AverageFixedTickDT_DataSet[32] = {};
        inline static std::mutex DT_Mutex;
        inline static std::mutex EventPolling_Mutex;
    };
};
