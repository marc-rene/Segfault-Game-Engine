#pragma once

#include <string>
#include "../Core/Log.hpp"
#include "TypeDefinitions.hpp"
#include "../Utilities/Utilities.hpp"
#include <string_view>

namespace ENGINE
{
    struct ENGINE_MODULE_INTERFACE
    {
    public:

        std::string_view Get_Module_Name() const
        {
            if (m_module_title.empty())
            {
                WARNc("No title was given for our module, assuming 'UNTITLED' for now");
                return "UNTITLED MODULE";
            }

            return m_module_title;
        }



        // -------------------------------------------------------------------------
        // --- Logging        ------------------------------------------------------

        void Trace(std::string_view message, std::format_args arguments = std::make_format_args())
        {
            auto msg = std::vformat(message, arguments);

            TRACE(std::string(m_module_title), "{}", msg);
        }

        void Info(std::string_view message, std::format_args arguments = std::make_format_args())
        {
            auto msg = std::vformat(message, arguments);

            INFO(std::string(m_module_title), "{}", msg);
        }

        void Warn(std::string_view message, std::format_args arguments = std::make_format_args())
        {
            auto msg = std::vformat(message, arguments);

            WARN(std::string(m_module_title), "{}", msg);
        }

        void Error(std::string_view message, std::format_args arguments = std::make_format_args())
        {
            auto msg = std::vformat(message, arguments);

            OhSHIT(std::string(m_module_title), "{}", msg);
        }

        // --- End of Logging ------------------------------------------------------
        // -------------------------------------------------------------------------

    protected:
        std::string_view m_module_title;
    };
}