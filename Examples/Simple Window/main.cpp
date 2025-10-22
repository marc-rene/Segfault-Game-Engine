#include <iostream>
#include <chrono>
#include <thread>


#include "Apocalypse.hpp"

#include "SDL3/SDL.h"

int main()
{
	ENGINE::Runtime::ClientRuntime clientRuntime;
	
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	return 0;
}