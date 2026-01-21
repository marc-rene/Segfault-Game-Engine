#include <iostream>
#include <chrono>
#include <thread>

#include "../../Include/Apocalypse.hpp"



int main()
{

    ENGINE::RUNTIME::ClientRuntime Runtime;
    
    Runtime.CreateWindow("GREAT SUCCESS", 1920, 1080);
    
    while (*Runtime.Is_Running() == true)
    {
        Runtime.Pre_tick();
    }
}