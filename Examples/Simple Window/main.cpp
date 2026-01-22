#include "Apocalypse.hpp"



int main()
{
    INFOc("All good to go!")
    
    ENGINE::RUNTIME::ClientRuntime Runtime;
    Runtime.Create_Window("Nothing to see here...", 1920, 1080);
    
    while (*Runtime.Is_Running() == true)
    {
        Runtime.Pre_tick();

        
    }
    
    return 0;
}

