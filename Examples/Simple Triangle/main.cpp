#include "Apocalypse.hpp"



int main()
{
    INFOc("All good to go!")
    
    ENGINE::RUNTIME::ClientRuntime Runtime;
    Runtime.Create_Window("You got a 'Point'...", 1920, 1080);

    while (*Runtime.Is_Running() == true)
    {
        Runtime.Pre_tick();
        
        Runtime.Tick();

        Runtime.Post_tick();
        
    }
    
    return 0;
}

