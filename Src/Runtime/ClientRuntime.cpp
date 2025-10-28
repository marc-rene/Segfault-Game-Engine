#include "ClientRuntime.hpp"

void ENGINE::Runtime::ClientRuntime::On_First_Ever_Frame()
{
    WARN("ENGINE::Runtime", "On_First_Ever_Frame() has never been defined just so you know");
}



void ENGINE::Runtime::ClientRuntime::On_FixedTick_Start()
{
	DT_Mutex.lock();
	// START THE CLOCK 
	FixedTickStartTimestamp = std::chrono::high_resolution_clock::now();
	DT_Mutex.unlock();

	if (ENGINE::Platform::WindowManager::PollEvents() == 0)
		Shutdown();
}



void ENGINE::Runtime::ClientRuntime::On_FixedTick_End()
{
	DT_Mutex.lock();
	// STOP THE CLOCK 
	auto FinishedTimestamp = std::chrono::high_resolution_clock::now();
	LastFixedTickDeltaTimeMicroSeconds = std::chrono::duration_cast<std::chrono::microseconds>(FinishedTimestamp - FixedTickStartTimestamp).count();
	if (LastFixedTickDeltaTimeMicroSeconds < 1) // We cant have 0 microseconds
		LastFixedTickDeltaTimeMicroSeconds = 1;
	
	DT_Mutex.unlock();
}