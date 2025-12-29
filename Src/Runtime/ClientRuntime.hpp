#pragma once

#include <chrono>
#include "TypeDefinitions.hpp"
#include "../Core/Log.hpp"
#include "../Platform/File Wizard.hpp"
#include "../Platform/Window.hpp"
#include "../Rendering/Rendering.hpp"
#include "../Utilities/Editor.hpp"

namespace ENGINE::Runtime
{

    class ClientRuntime
    {
    public:
        // Render ticks (1-1000 ticks a second)
        void On_First_Ever_Frame();
        void On_Frame_Start();		// early frame: events, paused?
        void On_Frame_End();		// cleanup, frame sync, present
        /*
        virtual void On_Frame();			// render submission
        virtual void On_Frame_UI();         // imgui / debug overlays

        virtual void On_ClientPaused();

        // Fixed ticks (60 ticks a second)
        virtual void On_First_Ever_FixedTick();
        */

        void On_FixedTick_Start();
        void On_FixedTick_Body() {};
        void On_FixedTick_End();

        void On_FixedTick();




        inline static void Shutdown()
        {
            INFO("ENGINE::Runtime::ClientRuntime", "SHUTTING DOWN ALL OPERATIONS");
            KeepRunning = false;
        }

        inline static bool IsRunning()
        {
            return KeepRunning;
        }

        bool EnableEditor()
        {
            return ENGINE::Editor::GetInstance()->Is_Initialised();
        }

        ClientRuntime()
        {
            FenceValue = 0;

            KeepRunning = true;
            ENGINE::Log::Init_Log();
            ENGINE::Log::Init_Err_Log();
            ENGINE::Settings::ActiveSettings::Initialise();

            auto windowManager_ref = ENGINE::Platform::WindowManager::GetInstance();
            windowManager_ref->CreateMainWindow();
        }

        inline static const long long GetFixedTickDeltaTimeMicroSeconds()
        {
            return LastFixedTickDeltaTimeMicroSeconds.count();
        }
        inline static const float GetFixedTickDeltaTimeMiliSeconds()
        {
            return LastFixedTickDeltaTimeMicroSeconds.count() / 1000.0f;
        }
        inline static const double GetFixedTickDeltaTimeMiliSeconds_double()
        {
            return LastFixedTickDeltaTimeMicroSeconds.count() / 1000.0;
        }

        inline static long long GetFixedTickDeltaTimeMicroSeconds_Average()
        {
            long long temp;
            temp = 0;
            for (int i = 0; i < 32; i++)
            {
                temp += AverageFixedTickDT_DataSet[i];
            }
            return (temp / 32);
        }

        ~ClientRuntime()
        {
            ENGINE::Platform::FileIO::Config::Flush();
        }
        

    private:
        const uMint FixedTickRate = 60;
        inline static unsigned long long FenceValue = 0;
        inline static bool KeepRunning = false;
        inline static std::chrono::steady_clock::time_point FixedTickStartTimestamp;
        inline static std::chrono::microseconds LastFixedTickDeltaTimeMicroSeconds;
        inline static long long AverageFixedTickDT_DataSet[32] = {};
        inline static std::mutex DT_Mutex;
        inline static std::mutex EventPolling_Mutex;




    };
};