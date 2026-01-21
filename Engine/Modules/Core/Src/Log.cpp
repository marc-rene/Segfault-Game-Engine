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

    void ENGINE::Log::Write(LogLevel level, std::string_view LoggerName, std::string_view message)
    {
        // Resolve logger name
        std::string name;
        if (LoggerName.empty())
        {
            name = (level == LogLevel::Error || level == LogLevel::Critical)
                     ? std::string(ERROR_LOG_NAME)
                     : std::string(DEFAULT_LOG_NAME);
        }
        else
        {
            name = std::string(LoggerName);
        }

        // Map to spdlog level and emit
        switch (level)
        {
        case LogLevel::Trace:    GetLogger(name)->log(spdlog::level::trace,    message); break;
        case LogLevel::Info:     GetLogger(name)->log(spdlog::level::info,     message); break;
        case LogLevel::Warn:     GetLogger(name)->log(spdlog::level::warn,     message); break;
        case LogLevel::Error:    GetLogger(name)->log(spdlog::level::err,      message); break;
        case LogLevel::Critical: GetLogger(name)->log(spdlog::level::critical, message); break;
        default:                 GetLogger(name)->log(spdlog::level::info,     message); break;
        }
    }

} //end Engine Namespace
