#include "../include/FileWizard.hpp"

int LINK_TEST_FileWizard()
{
    try
    {
        mINI::INIStructure LINK_TEST_mINI;
        simdjson::ondemand::parser LINK_TEST_Json;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return -1;
    }

    return 43;
}
