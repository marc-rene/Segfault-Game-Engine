#pragma once

#include <string>
#include "Log.hpp"
#include "TypeDefinitions.hpp"
#include "Utilities.hpp"
#include <string_view>

namespace ENGINE
{
    struct ENGINE_MODULE_INTERFACE
    {
    public:
        ENGINE_MODULE_INTERFACE(std::string p_module_title) : m_module_title{p_module_title}
        {
        };


        std::string Get_Module_Name() const
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

        virtual void Trace(std::string_view message, std::format_args arguments = std::make_format_args())
        {
            auto msg = std::vformat(message, arguments);

            TRACE(std::string(m_module_title), "{}", msg);
        }

        virtual void Info(std::string_view message, std::format_args arguments = std::make_format_args())
        {
            auto msg = std::vformat(message, arguments);

            INFO(std::string(m_module_title), "{}", msg);
        }

        virtual void Warn(std::string_view message, std::format_args arguments = std::make_format_args())
        {
            auto msg = std::vformat(message, arguments);

            WARN(std::string(m_module_title), "{}", msg);
        }

        virtual void Error(std::string_view message, std::format_args arguments = std::make_format_args())
        {
            auto msg = std::vformat(message, arguments);

            OhSHIT(std::string(m_module_title), "{}", msg);
        }

        virtual void Critical(std::string_view message, std::format_args arguments = std::make_format_args())
        {
            auto msg = std::vformat(message, arguments);

            CRITICAL("{}", msg);
        }

        // --- End of Logging ------------------------------------------------------
        // -------------------------------------------------------------------------

        std::string m_module_title;
    };
}
