#include <string>
#include "../Core/Log.hpp"

struct ENGINE_MODULE
{
public:

    explicit ENGINE_MODULE(std::string_view Module_Title) : m_MODULE_TITLE(Module_Title) {}

    void Trace(std::string_view message, std::format_args arguments = std::make_format_args())
    {
        auto msg = std::vformat(message, arguments);

        TRACE(std::string(m_MODULE_TITLE), "{}", msg);
    }

    void Info(std::string_view message, std::format_args arguments = std::make_format_args())
    {
        auto msg = std::vformat(message, arguments);

        INFO(std::string(m_MODULE_TITLE), "{}", msg);
    }

    void Warn(std::string_view message, std::format_args arguments = std::make_format_args())
    {
        auto msg = std::vformat(message, arguments);

        WARN(std::string(m_MODULE_TITLE), "{}", msg);
    }

    void Error(std::string_view message, std::format_args arguments = std::make_format_args())
    {
        auto msg = std::vformat(message, arguments);

        OhSHIT(std::string(m_MODULE_TITLE), "{}", msg);
    }

private:
    std::string_view m_MODULE_TITLE;
};
