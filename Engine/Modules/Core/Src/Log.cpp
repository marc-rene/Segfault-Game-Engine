#include "../Include/Log.hpp"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/async.h"
#include <filesystem>

#define SPDLOG_USE_STD_FORMAT



// This is where the fun begins!
namespace ENGINE
{
    std::shared_ptr<spdlog::logger> GetLogger(std::string LoggerName)
    {
        if (Log::is_first_init())
        {
            Log::Init_Log(DEFAULT_LOG_NAME);
        }

        if (spdlog::get(LoggerName) == nullptr)
        {
            WARNc("HEY! {} DOESN'T EXIST YET... Making him now, Logs are saved at {} too", LoggerName,
                  std::filesystem::current_path().string());

            Log::Init_Log(LoggerName);
        }
        return spdlog::get(LoggerName);
    }


    void Log::Init_Log()
    {
        Init_Log(DEFAULT_LOG_NAME);
    }

    
    void Log::Init_Err_Log()
    {
        Init_Log(ERROR_LOG_NAME);
    }
    
    
    /// @param LoggerName What is the name of our new logger? "File Wizard"? "DaVinci"? "DirectX helper"? "Jesus"?
    void Log::Init_Log(std::string LoggerName)
    {
        try
        {
            // Thanks https://www.w3schools.com/cpp/trycpp.asp?filename=demo_date_strftime
            static time_t timestamp = time(nullptr);
            static struct tm datetime = *localtime(&timestamp);
            static char formatted_date[50];
            strftime(formatted_date, 50, "%e %b %H-%M", &datetime);

            static auto frontEndConsoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

            // Console logs will follow this pattern
            // -> [15:42 59s] [File Wizard] [info]    Hello :)
            frontEndConsoleSink->set_pattern("-> [%H:%M %Ss]  %n: \t%^[%l]\t %v%$");
                        
            frontEndConsoleSink->set_level(spdlog::level::level_enum(LowestAllowedLevel));

            // Logs will save to "Engine/logs/Engine 2026-20-01.log"
            static std::string filename = std::format(
                "{}/logs/{} {}.log", std::filesystem::current_path().string(), ENGINE_NAME, formatted_date);
            
            static auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, true);
            
            // 4 was chosen at random
            spdlog::flush_every(std::chrono::seconds(4));
            
            std::vector<spdlog::sink_ptr> sinks{frontEndConsoleSink, fileSink};

            // ---------------------
            // ASYNC IMPLEMENTATION | TODO: Fix
            // ---------------------
            //if (total_inits == 0)
            //    spdlog::init_thread_pool(8192, 1);
            //auto logger = std::make_shared<spdlog::async_logger>(LoggerName, sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::overrun_oldest);
            
            auto logger = std::make_shared<spdlog::logger>(LoggerName, sinks.begin(), sinks.end());
            
            // by default this is set to like 1 which is debug / trace
            // Change this anytime using SetLoggerVerbosity(int)
            logger->set_level(spdlog::level::level_enum(LowestAllowedLevel)); 
            
            spdlog::register_logger(logger);

            first_init = false;
        }

        catch (const spdlog::spdlog_ex& ex) // Logger probably already exists
        {
            if (first_init)
            {
                std::cout << "Hey, Just so you know, we got SPDLOG tomfoolery here because " << ex.what() << '\n';
            }
            else
            {
                WARNc("We got SPDLOG tomfoolery here because {}", ex.what());
            }
        }
    }


    /// @param LoggerName Name of logger to use... if you don't know you can use CRITICAL log macro instead
    /// @param fmtStr The string that your arguments will go into using '{}'  eg: ("Salut {} {}", "Maman", 5) 
    /// @param args What arguemnts do you want to put into your string?
    template <class... Args>
    void Log::Trace(std::string LoggerName, std::format_string<Args...> fmtStr, Args&&... args)
    {
        if (LoggerName.empty())
        {
            GetLogger(DEFAULT_LOG_NAME)->trace(fmtStr, std::forward<Args>(args)...);
        }
        else
        {
            GetLogger(LoggerName)->trace(fmtStr, std::forward<Args>(args)...);
        }
    }


    /// @param LoggerName Name of logger to use... if you don't know you can use CRITICAL log macro instead
    /// @param fmtStr The string that your arguments will go into using '{}'  eg: ("Salut {} {}", "Maman", 5) 
    /// @param args What arguemnts do you want to put into your string?
    template <class... Args>
    void Log::Info(std::string LoggerName, std::format_string<Args...> fmtStr, Args&&... args)
    {
        if (LoggerName.empty())
        {
            GetLogger(DEFAULT_LOG_NAME)->info(fmtStr, std::forward<Args>(args)...);
        }
        else
        {
            GetLogger(LoggerName)->info(fmtStr, std::forward<Args>(args)...);
        }
    }


    /// @param LoggerName Name of logger to use... if you don't know you can use CRITICAL log macro instead
    /// @param fmtStr The string that your arguments will go into using '{}'  eg: ("Salut {} {}", "Maman", 5) 
    /// @param args What arguemnts do you want to put into your string?
    template <class... Args>
    void Log::Warn(std::string LoggerName, std::format_string<Args...> fmtStr, Args&&... args)
    {
        if (LoggerName.empty())
        {
            GetLogger(DEFAULT_LOG_NAME)->warn(fmtStr, std::forward<Args>(args)...);
        }
        else
        {
            GetLogger(LoggerName)->warn(fmtStr, std::forward<Args>(args)...);
        }
    }


    /// @param LoggerName Name of logger to use... you can use CRITICAL log macro instead, or leave nullptr
    /// @param fmtStr The string that your arguments will go into using '{}'  eg: ("Salut {} {}", "Maman", 5)
    /// @param args What arguemnts do you want to put into your string?
    template <class... Args>
    void Log::Error(std::string LoggerName, std::format_string<Args...> fmtStr, Args&&... args)
    {
        if (LoggerName.empty())
        {
            GetLogger(ERROR_LOG_NAME)->error(fmtStr, std::forward<Args>(args)...);
        }
        else
        {
            GetLogger(LoggerName)->error(fmtStr, std::forward<Args>(args)...);
        }
    }


    /// @param LoggerName Name of logger to use... if you don't know you can use CRITICAL log macro instead
    /// @param fmtStr The string that your arguments will go into using '{}'  eg: ("Salut {} {}", "Maman", 5) 
    /// @param args What arguemnts do you want to put into your string?
    template <class... Args>
    void Log::Critical(std::string LoggerName, std::format_string<Args...> fmtStr, Args&&... args)
    {
        if (LoggerName.empty())
        {
            GetLogger(ERROR_LOG_NAME)->critical(fmtStr, std::forward<Args>(args)...);
        }
        else
        {
            GetLogger(LoggerName)->critical(fmtStr, std::forward<Args>(args)...);
        }
    }


    /// @brief What verbosity do we want to update all our logs to?
    /// @param NewLevel 1: Debugging / Trace, 2: General Info, 3: Warnings, 4: Errors, 5: APOCALYPSE
    /// @return True if success
    bool Log::SetLoggerVerbosity(int NewLevel)
    {
        LowestAllowedLevel = std::clamp(NewLevel, 1, 5);

        try
        {
            spdlog::set_level(spdlog::level::level_enum(LowestAllowedLevel));
            return true;
        }
        catch (std::exception& ex)
        {
            WARNc("Couldn't change logger verbsoity to {} because {}", LowestAllowedLevel, ex.what());
            return false;
        }
    } // end SetLoggerVerbosity
    
} //end Engine Namespace
