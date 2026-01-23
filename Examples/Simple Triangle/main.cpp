#include "Apocalypse.hpp"



int main()
{
    INFOc("All good to go!")
    
    ENGINE::RUNTIME::ClientRuntime Runtime;
    Runtime.Create_Window("You got a 'Point'...", 1920, 1080);
    
    // Change Background
    Runtime.Get_DaVinci_instance()->Set_Clear_Colour(0.7f, 0.2f, 0.2f, 1.0f);
    
    while (*Runtime.Is_Running() == true)
    {
        Runtime.Pre_tick();
        
        Runtime.Tick();

        Runtime.Post_tick();
    }
    
    return 0;
}

