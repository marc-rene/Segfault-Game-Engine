#pragma once

#include <chrono>
#include "TypeDefinitions.hpp"
#include "../Platform/File Wizard.hpp"
#include "../Platform/Window.hpp"


namespace ENGINE::Runtime
{

	class ClientRuntime
	{
	public:
		// Render ticks (1-1000 ticks a second)
		void On_First_Ever_Frame();
		/*
		virtual void On_Frame_Start();		// early frame: events, paused?
		virtual void On_Frame();			// render submission
		virtual void On_Frame_UI();         // imgui / debug overlays
		virtual void On_Frame_End();		// cleanup, frame sync, present

		virtual void On_ClientPaused();

		// Fixed ticks (60 ticks a second)
		virtual void On_First_Ever_FixedTick();
		*/

		void On_FixedTick_Start();
		void On_FixedTick() {};
		void On_FixedTick_End();

		inline static void Shutdown()
		{
			INFO("ENGINE::Runtime::ClientRuntime", "SHUTTING DOWN ALL OPERATIONS");
			KeepRunning = false;
		}
		
		bool IsRunning()
		{
			return KeepRunning;
		}

		ClientRuntime()
		{
			KeepRunning = true;
			ENGINE::Log::Init_Log();
			ENGINE::Log::Init_Err_Log();
			ENGINE::Settings::ActiveSettings::Initialise();
			
			auto windowManager_ref = ENGINE::Platform::WindowManager::GetInstance();
			windowManager_ref->CreateMainWindow();
		
		}

		inline static const long long GetFixedTickDeltaTimeMicroSeconds()
		{
			return LastFixedTickDeltaTimeMicroSeconds;
		}
		inline static const float GetFixedTickDeltaTimeMiliSeconds()
		{
			return LastFixedTickDeltaTimeMicroSeconds / 1000.0f;
		}
		inline static const double GetFixedTickDeltaTimeMiliSeconds_double()
		{
			return LastFixedTickDeltaTimeMicroSeconds / 1000.0;
		}
		

		~ClientRuntime()
		{
			ENGINE::Platform::FileIO::Config::Flush();
		}
	private:
		const uMint FixedTickRate = 60;
		inline static bool KeepRunning = false;
		inline static std::chrono::steady_clock::time_point FixedTickStartTimestamp;
		inline static long long LastFixedTickDeltaTimeMicroSeconds;
		inline static std::mutex DT_Mutex;
		

	};
};