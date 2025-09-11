#include "../include/FileWizard.hpp"
#include "../mINI/src/mini/ini.h"
#include "../JSON/include/simdjson.h"

int LINK_TEST_FileWizard()
{
    try
    {
        mINI::INIStructure LINK_TEST_mINI;
        simdjson::ondemand::parser LINK_TEST_Json;
        fw_INFO(SUCCESS_msg);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return -1;
    }

    return 43;
}
