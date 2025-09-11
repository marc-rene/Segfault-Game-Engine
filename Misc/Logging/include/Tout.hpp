#pragma once

#include <iostream>
#include <memory>
#include "spdlog/spdlog.h"
#include "spdlog/stopwatch.h"
#include "spdlog/sinks/callback_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/async.h"

#include <filesystem>

#define log_errorLogName "TOMFOOLERY"
#define SPDLOG_LEVEL trace

namespace TOUT_LOG
{

	static std::shared_ptr<spdlog::logger> Init_Log();
	static std::shared_ptr<spdlog::logger> Init_Err_Log();
	static std::shared_ptr<spdlog::logger> Init_Log(const char* LoggerName);

	static std::shared_ptr<spdlog::logger> GetLogger();
	static std::shared_ptr<spdlog::logger> GetLogger(std::string LoggerName);
	static std::shared_ptr<spdlog::logger> GetLogger(const char* LoggerName);

}

#define TRACEc(...)                                                                                       \
    {                                                                                                     \
        try                                                                                               \
        {                                                                                                 \
            TOUT_LOG::GetLogger()->trace(__VA_ARGS__);                                                    \
        }                                                                                                 \
        catch (const std::exception &e)                                                                   \
        {                                                                                                 \
            std::cout << std::format("\n! Trace log error: {}!\n", e.what()) << std::format(__VA_ARGS__); \
        }                                                                                                 \
    }
#define TRACE(logger_name, ...)                                                                          \
    {                                                                                                    \
        try                                                                                              \
        {                                                                                                \
            TOUT_LOG::GetLogger(logger_name)->trace(__VA_ARGS__);                                        \
        }                                                                                                \
        catch (const std::exception &e)                                                                  \
        {                                                                                                \
            std::cout << std::format("\n!Trace log error: {}!\n", e.what()) << std::format(__VA_ARGS__); \
        }                                                                                                \
    }

#define INFOc(...)                                                                                          \
    {                                                                                                       \
        try                                                                                                 \
        {                                                                                                   \
            TOUT_LOG::GetLogger()->info(__VA_ARGS__);                                                       \
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
            TOUT_LOG::GetLogger(logger_name)->info(__VA_ARGS__);                                            \
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
            TOUT_LOG::GetLogger()->warn(__VA_ARGS__);                                                       \
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
            TOUT_LOG::GetLogger(logger_name)->warn(__VA_ARGS__);                                            \
        }                                                                                                   \
        catch (const std::exception &e)                                                                     \
        {                                                                                                   \
            std::cout << std::format("\n!!!WARN LOG ERROR: {}!!!\n", e.what()) << std::format(__VA_ARGS__); \
        }                                                                                                   \
    }

#define OhSHITc(...)                                                                                          \
    {                                                                                                        \
        try                                                                                                  \
        {                                                                                                    \
            TOUT_LOG::GetLogger(log_errorLogName)->error(__VA_ARGS__);                                       \
        }                                                                                                    \
        catch (const std::exception &e)                                                                      \
        {                                                                                                    \
            std::cout << std::format("\n!!!FUCK FUCK ERROR: {}!!!\n", e.what()) << std::format(__VA_ARGS__); \
        }                                                                                                    \
    }

// MACRO Redefinition - TODO: Find cool new memorable name
#define OhSHIT(logger_name, ...)                                                                              \
	{                                                                                                        \
		try                                                                                                  \
		{                                                                                                    \
			TOUT_LOG::GetLogger(logger_name)->error(__VA_ARGS__);                                            \
		}                                                                                                    \
		catch (const std::exception &e)                                                                      \
		{                                                                                                    \
			std::cout << std::format("\n!!!FUCK FUCK ERROR: {}!!!\n", e.what()) << std::format(__VA_ARGS__); \
		}                                                                                                    \
	}


#define CRITICAL(...)                                                                                                                 \
    {                                                                                                                                 \
        try                                                                                                                           \
        {                                                                                                                             \
            TOUT_LOG::GetLogger(log_errorLogName)->critical(__VA_ARGS__);                                                             \
            std::cerr << std::format("\n!!!CHORNOBYL REACTOR 4\nCHORNOBYL REACTOR 4: {}!!!\n", e.what()) << std::format(__VA_ARGS__); \
            std::cout << std::format("\n!!!CHORNOBYL REACTOR 4\nCHORNOBYL REACTOR 4: {}!!!\n", e.what()) << std::format(__VA_ARGS__); \
        }                                                                                                                             \
        catch (const std::exception &)                                                                                                \
        {                                                                                                                             \
            std::cout << std::format("\n!!!THINGS ARE SO BAD WE CANT EVEN SAY OUR CRITICAL BECAUSE: {}!!!\n", e.what());              \
        }                                                                                                                             \
    }

#define SUCCESS_msg "GREAT SUCESS"
#define WARNING_msg "Oh Wawaweewa..."
#define FAILURE_msg "PAIN IN MY ASSHOLES"
#define FAREWELL_msg "Good Hunting S.t.a.l.k.e.r"

#define TIMER_START spdlog::stopwatch sw
#define TIMER_ELAPSEDc(TimerName, ...) TOUT_LOG::GetLogger(TimerName)->trace(__VA_ARGS__, sw) // Usage: TIMER_ELAPSEDc("Taken {:.3) seconds so far!)

#define TIMER_ELAPSED(logger_name, ...) TOUT_LOG::A_LIFE_Log::GetLogger(logger_name)->trace(__VA_ARGS__, sw) // Usage: TIMER_ELAPSEDc("Taken {:.3) seconds so far!)


/*
I deeply apologise for this inline mess

Visual studio was giving me definition errors because of how this logger comes first and random tomfoolery with SPDLOG

The easiest thing to do is copy Tout.cpp into the .hpp and define everything inline

I apologise...

*/



inline int LINK_TEST_Logger()
{
    try
    {
        spdlog::info("Logger Success!");
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return -1;
    }

    return 43;
}

namespace TOUT_LOG
{
    std::shared_ptr<spdlog::logger> Init_Log()
    {
        return Init_Log(PROJECT_NAME);
    }

    std::shared_ptr<spdlog::logger> Init_Err_Log()
    {
        try
        {
            std::shared_ptr<spdlog::logger> errorLogger = spdlog::stdout_color_mt(log_errorLogName);
            spdlog::set_error_handler([](const std::string& msg)
                {
                    spdlog::get(log_errorLogName)->error("*** {} ***: {}", FAILURE_msg, msg);
                });

            return errorLogger;
        }
        catch (const spdlog::spdlog_ex&)
        {
            return spdlog::get(log_errorLogName);
        }
    }

    std::shared_ptr<spdlog::logger> Init_Log(const char* LoggerName)
    {
        if (Init_Err_Log() == nullptr)
        {
            //todo: Add error log init failure catch here
        }

        try
        {
            // Thanks https://www.w3schools.com/cpp/trycpp.asp?filename=demo_date_strftime
            static time_t timestamp = time(nullptr);
            static struct tm datetime = *localtime(&timestamp);
            static char formatted_date[50];
            strftime(formatted_date, 50, "%e %b %H-%M", &datetime);

            static auto frontEndConsoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            
            frontEndConsoleSink->set_pattern("-> [%H:%M %Ss]  %n: \t%^[%l]\t %v%$"); // -> [15:42 59s] [File Wizard] [info]    Hello :)
            
            static std::string filename = std::format("{}/logs/{} {}.log", std::filesystem::current_path().string(), PROJECT_NAME, formatted_date);
            static auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, true);
            spdlog::flush_every(std::chrono::seconds(4));
            std::vector<spdlog::sink_ptr> sinks{ frontEndConsoleSink, fileSink };

            // ---------------------
            // ASYNC IMPLEMENTATION | TODO: Fix
            // ---------------------
            //if (total_inits == 0)
            //    spdlog::init_thread_pool(8192, 1);
            //auto logger =       std::make_shared<spdlog::async_logger>(LoggerName, sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::overrun_oldest);

            auto logger = std::make_shared<spdlog::logger>(LoggerName, sinks.begin(), sinks.end());
            logger->set_level(spdlog::level::SPDLOG_LEVEL); // or info/warn
            spdlog::register_logger(logger);

            return logger;
        }
        catch (const spdlog::spdlog_ex& ex) // Logger probably already exists
        {
            std::cout << "Hey, Just so you know, we got SPDLOG tomfoolery here because " << ex.what() << std::endl;
            return spdlog::get(LoggerName);
        }
    }

    std::shared_ptr<spdlog::logger> GetLogger()
    {
        return GetLogger(PROJECT_NAME);
    }

    std::shared_ptr<spdlog::logger> GetLogger(const std::string LoggerName)
    {
        return GetLogger(LoggerName.c_str());
    }

    std::shared_ptr<spdlog::logger> GetLogger(const char* LoggerName)
    {
        // Use Init instead because if the logger doesn't exist we'll make one..very sketch
        if (spdlog::get(LoggerName) == nullptr)
        {
            WARNc("HEY! {} DOESN'T EXIST YET... Making him now, Logs are saved at {} too", LoggerName,
                std::filesystem::current_path().string());
            Init_Log(LoggerName);
        }

        return spdlog::get(LoggerName);
    }
}
