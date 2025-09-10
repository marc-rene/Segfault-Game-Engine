#include "Log.hpp"
#include "FileWizard.hpp"
#include "PacketNinja.hpp"

int main()
{
    int linkResult = LINK_TEST_Logger() + LINK_TEST_FileWizard() + LINK_TEST_PacketNinja();
    int expected = 43 * 3;
    return linkResult == expected;
}