#pragma once

#include <iostream>
#include <format>
#include <string>
#include <string_view>
#include <utility>

#define DEFAULT_LOG_NAME    "Toute"
#define ERROR_LOG_NAME      "TOMFOOLERY"
#define SPDLOG_LEVEL        trace

#define SUCCESS_msg     "GREAT SUCESS"
#define WARNING_msg     "Oh Wawaweewa..."
#define FAILURE_msg     "PAIN IN MY ASSHOLES"
#define FAREWELL_msg    "Good Hunting S.T.A.L.K.E.R"

namespace ENGINE
{
    enum class LogLevel { Trace, Info, Warn, Error, Critical };

    struct Log
    {
        static void Init_Log();
        static void Init_Log(std::string LoggerName);
        static void Init_Err_Log();

        inline static bool is_first_init() { return first_init; }

        /// @brief What verbosity do we want to update all our logs to?
        /// @param NewLevel 1: Debugging / Trace, 2: General Info, 3: Warnings, 4: Errors, 5: APOCALYPSE
        /// @return True if success
        static bool SetLoggerVerbosity(int NewLevel);

        // Non-template sink (implemented in Log.cpp)
        static void Write(LogLevel level, std::string_view LoggerName, std::string_view message);

        // Header-only wrappers (templates MUST live in the header)
        template <class... Args>
        static void Trace(std::string_view LoggerName, std::format_string<Args...> fmtStr, Args&&... args)
        {
            const std::string_view name = LoggerName.empty() ? std::string_view(DEFAULT_LOG_NAME) : LoggerName;
            Write(LogLevel::Trace, name, std::format(fmtStr, std::forward<Args>(args)...));
        }

        template <class... Args>
        static void Info(std::string_view LoggerName, std::format_string<Args...> fmtStr, Args&&... args)
        {
            const std::string_view name = LoggerName.empty() ? std::string_view(DEFAULT_LOG_NAME) : LoggerName;
            Write(LogLevel::Info, name, std::format(fmtStr, std::forward<Args>(args)...));
        }

        template <class... Args>
        static void Warn(std::string_view LoggerName, std::format_string<Args...> fmtStr, Args&&... args)
        {
            const std::string_view name = LoggerName.empty() ? std::string_view(DEFAULT_LOG_NAME) : LoggerName;
            Write(LogLevel::Warn, name, std::format(fmtStr, std::forward<Args>(args)...));
        }

        template <class... Args>
        static void Error(std::string_view LoggerName, std::format_string<Args...> fmtStr, Args&&... args)
        {
            const std::string_view name = LoggerName.empty() ? std::string_view(ERROR_LOG_NAME) : LoggerName;
            Write(LogLevel::Error, name, std::format(fmtStr, std::forward<Args>(args)...));
        }

        template <class... Args>
        static void Critical(std::string_view LoggerName, std::format_string<Args...> fmtStr, Args&&... args)
        {
            const std::string_view name = LoggerName.empty() ? std::string_view(ERROR_LOG_NAME) : LoggerName;
            Write(LogLevel::Critical, name, std::format(fmtStr, std::forward<Args>(args)...));
        }

    private:
        inline static bool first_init = true;
        inline static int LowestAllowedLevel = 0;
    };
}

#define TRACEc(...)                                                                                         \
    {                                                                                                       \
        try                                                                                                 \
        {                                                                                                   \
            ENGINE::Log::Trace(DEFAULT_LOG_NAME, __VA_ARGS__);                                              \
        }                                                                                                   \
        catch (const std::exception &e)                                                                     \
        {                                                                                                   \
            std::cout << std::format("\n! Trace log error: {}!\n", e.what()) << std::format(__VA_ARGS__);   \
        }                                                                                                   \
    }

#define TRACE(logger_name, ...)                                                                             \
    {                                                                                                       \
        try                                                                                                 \
        {                                                                                                   \
            ENGINE::Log::Trace(logger_name, __VA_ARGS__);                                                   \
        }                                                                                                   \
        catch (const std::exception &e)                                                                     \
        {                                                                                                   \
            std::cout << std::format("\n!Trace log error: {}!\n", e.what()) << std::format(__VA_ARGS__);    \
        }                                                                                                   \
    }

#define INFOc(...)                                                                                          \
    {                                                                                                       \
        try                                                                                                 \
        {                                                                                                   \
            ENGINE::Log::Info(DEFAULT_LOG_NAME, __VA_ARGS__);                                               \
        }                                                                                                   \
        catch (const std::exception &e)                                                                     \
        {                                                                                                   \
            std::cout << std::format("\n!!!INFO LOG ERROR: {}!!!\n", e.what()) << std::format(__VA_ARGS__); \
        }                                                                                                   \
    }
#define INFO(logger_name, ...)                                                                              \
    {                                                                                                       \
        try                                                                                                 \
        {                                                                                                   \
            ENGINE::Log::Info(logger_name, __VA_ARGS__);                                                    \
        }                                                                                                   \
        catch (const std::exception &e)                                                                     \
        {                                                                                                   \
            std::cout << std::format("\n!!!INFO LOG ERROR: {}!!!\n", e.what()) << std::format(__VA_ARGS__); \
        }                                                                                                   \
    }
#define WARNc(...)                                                                                          \
    {                                                                                                       \
        try                                                                                                 \
        {                                                                                                   \
            ENGINE::Log::Warn(DEFAULT_LOG_NAME, __VA_ARGS__);                                               \
        }                                                                                                   \
        catch (const std::exception &e)                                                                     \
        {                                                                                                   \
            std::cout << std::format("\n!!!WARN LOG ERROR: {}!!!\n", e.what()) << std::format(__VA_ARGS__); \
        }                                                                                                   \
    }
#define WARN(logger_name, ...)                                                                              \
    {                                                                                                       \
        try                                                                                                 \
        {                                                                                                   \
            ENGINE::Log::Warn(logger_name, __VA_ARGS__);                                                    \
        }                                                                                                   \
        catch (const std::exception &e)                                                                     \
        {                                                                                                   \
            std::cout << std::format("\n!!!WARN LOG ERROR: {}!!!\n", e.what()) << std::format(__VA_ARGS__); \
        }                                                                                                   \
    }

#define OhSHITc(...)                                                                                        \
    {                                                                                                       \
        try                                                                                                 \
        {                                                                                                   \
            ENGINE::Log::Error(ERROR_LOG_NAME, __VA_ARGS__);                                                \
        }                                                                                                   \
        catch (const std::exception &e)                                                                     \
        {                                                                                                   \
            std::cout << std::format("\n!!!FUCK FUCK ERROR: {}!!!\n", e.what()) << std::format(__VA_ARGS__);\
        }                                                                                                   \
    }

// MACRO Redefinition - TODO: Find cool new memorable name
#define OhSHIT(logger_name, ...)                                                                            \
	{                                                                                                       \
		try                                                                                                 \
		{                                                                                                   \
			ENGINE::Log::Error(logger_name, __VA_ARGS__);                                                   \
		}                                                                                                   \
		catch (const std::exception &e)                                                                     \
		{                                                                                                   \
			std::cout << std::format("\n!!!FUCK FUCK ERROR: {}!!!\n", e.what()) << std::format(__VA_ARGS__);\
		}                                                                                                   \
	}

#define CRITICAL(...)                                                                                               \
    {                                                                                                               \
        try                                                                                                         \
        {                                                                                                           \
            ENGINE::Log::Critical(ERROR_LOG_NAME, __VA_ARGS__);                                                     \
            std::cerr << std::format("\n!!!CHORNOBYL REACTOR 4\nCHORNOBYL REACTOR 4:") << std::format(__VA_ARGS__); \
            std::cout << std::format("\n!!!CHORNOBYL REACTOR 4\nCHORNOBYL REACTOR 4:") << std::format(__VA_ARGS__); \
        }                                                                                                           \
        catch (const std::exception &e)                                                                             \
        {                                                                                                           \
            std::cout << std::format("\n!!!THINGS ARE SO BAD WE CANT EVEN SAY OUR CRITICAL BECAUSE: {}!!!\n", e.what());\
        }                                                                                                           \
    }
