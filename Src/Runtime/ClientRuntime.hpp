#pragma once

#include "TypeDefinitions.hpp"
#include "../Platform/File Wizard.hpp"
#include "../Platform/Window.hpp"


namespace ENGINE::Runtime
{

	class ClientRuntime
	{
	public:
		/*
		// Render ticks (1-1000 ticks a second)
		virtual void On_First_Ever_Frame();
		virtual void On_Frame_Start();		// early frame: events, paused?
		virtual void On_Frame();			// render submission
		virtual void On_Frame_UI();         // imgui / debug overlays
		virtual void On_Frame_End();		// cleanup, frame sync, present

		virtual void On_ClientPaused();

		// Fixed ticks (60 ticks a second)
		virtual void On_First_Ever_FixedTick();
		virtual void On_FixedTick_Start();
		virtual void On_FixedTick();
		virtual void On_FixedTick_End();

		virtual void On_Shutdown();
		*/
		
		ClientRuntime()
		{
			ENGINE::Log::Init_Log();
			ENGINE::Log::Init_Err_Log();
			ENGINE::Platform::WindowManager ClientWindowManager;

			ClientWindowManager.Initialise();

			ENGINE::Settings::WindowSettings windowsSettings;

			ClientWindowManager.CreateMainWindow(&windowsSettings);
		}

		~ClientRuntime()
		{
			ENGINE::Platform::FileIO::Config::Flush();
		}
	private:
		//const uMint FixedTickRate = 60;

	};
};