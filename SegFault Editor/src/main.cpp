#include "Tout.hpp"
#include "PacketNinja.hpp"
#include "FileWizard.hpp"



int main()
{
    TOUT_LOG::Init_Log(); // Make SURE this always is launched first

    // Linker Sanity check
    int linkResult = LINK_TEST_Logger() + LINK_TEST_FileWizard() + LINK_TEST_PacketNinja();
    
    
    return linkResult == (43 * 3);
}