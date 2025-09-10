#include "../include/Log.hpp"

int LINK_TEST_Logger()
{
    try
    {
        spdlog::info("SUCCESS");
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return -1;
    }

    return 43;
}