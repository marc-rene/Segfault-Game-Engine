#include <iostream>
#include <cstdint>

#include "Apocalypse.hpp"



int main()
{
	ENGINE::Runtime::ClientRuntime Runtime;

	ENGINE::Platform::FileIO::Config::SetSetting("Window", "Width", "1280");
	ENGINE::Platform::FileIO::Config::SetSetting("Window", "Height", "700");
	ENGINE::Platform::FileIO::Config::SetSetting("Window", "Width", "2000");

	return 0;
}