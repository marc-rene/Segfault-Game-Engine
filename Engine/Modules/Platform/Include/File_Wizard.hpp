#pragma once

#include "TypeDefinitions.hpp"
#include "Log.hpp"
#include "../Src/ini.h"
#include <filesystem>
#include <mutex>

#define CONFIG_FILE_NAME "EngineConfig.ini"



namespace ENGINE::Platform::FileIO
{
    struct Config
    {
    public:
        inline static bool SetSetting(std::string Section, std::string Key, std::string value)
        {
            if (ready == false)
                initialise();

            mutex_lock config_lock(mut_config_lock);
            bool overwriting = ini_structure.get(Section).has(value);

            ini_structure[Section][Key] = value;

            TRACEc("Runtime Config {} [{}] [{}] to {}", overwriting ? "overwriting" : "set", Section, Key, value);
            return true;
        }

        inline static bool GetSetting(std::string Section, std::string Key, std::string& destination, bool createIfNotExist = true, std::string default_str = "NaN")
        {
            if (ready == false)
                initialise();

            mutex_lock config_lock(mut_config_lock);

            if (ini_structure.get(Section).has(Key))  // exists?
            {
                destination = ini_structure.get(Section).get(Key);
                return true;
            }
            else
            {
                if (createIfNotExist)
                {
                    ini_structure[Section][Key] = default_str;
                    destination = default_str;
                    return false;
                }
                return false;
            }
        }



        inline static mint GetSetting_mint(std::string Section, std::string Key, mint Default = 0)
        {
            std::string temp_str;
            if ( GetSetting(Section, Key, temp_str, true, std::to_string((int)Default)))
            {
                try
                {
                    mint result = std::stoi(temp_str);
                    return result;
                }
                catch (const std::exception& e)
                {
                    WARNc("Couldn't find value {} because {}\n so will set it to default: {}", Key, e.what(), std::to_string((int)Default));
                    return Default;
                }
            }
            else
            {
                WARNc("Couldn't find {} so we're setting as default: {}", Key, std::to_string((int)Default));
                return Default;
            }
        }



        inline static int GetSetting_int(std::string Section, std::string Key, int Default = 0)
        {
            std::string temp_str;
            if (GetSetting(Section, Key, temp_str, true, std::to_string(Default)))
            {
                try
                {
                    int result = std::stoi(temp_str);
                    return result;
                }
                catch (const std::exception& e)
                {
                    WARNc("Couldn't find value {} because {}\n so will set it to default: {}", Key, e.what(), Default);
                    return Default;
                }
            }
            else
            {
                WARNc("Couldn't find {} so we're setting as default: {}", Key, Default);
                return Default;
            }
        }


        inline static float GetSetting_float(std::string Section, std::string Key, float Default = 0.0f)
        {
            std::string temp_str;
            if (GetSetting(Section, Key, temp_str, true, std::to_string(Default)))
            {
                try
                {
                    float result = std::stof(temp_str);
                    return result;
                }
                catch (const std::exception& e)
                {
                    WARNc("Couldn't find value {} because {}\n so will set it to default: {}", Key, e.what(), Default);
                    return Default;
                }
            }
            else
            {
                WARNc("Couldn't find {} so we're setting as default: {}", Key, Default);
                return Default;
            }
        }



        inline static std::string GetSetting_str(std::string Section, std::string Key, std::string Default = "NaN")
        {
            std::string temp_str;
            if (GetSetting(Section, Key, temp_str, true, Default))
            {
                try
                {
                    return temp_str;
                }
                catch (const std::exception& e)
                {
                    WARNc("Couldn't find value {} because {}\n so will set it to default: {}", Key, e.what(), Default);
                    return Default;
                }
            }
            else
            {
                WARNc("Couldn't find {} so we're setting as default: {}", Key, Default);
                return Default;
            }
        }

        inline static void Flush() // Make sure that we dont have the entire Config in memory at all times
        {
            mutex_lock config_lock(mut_config_lock);
            ini_config_ref->write(ini_structure);
            ini_structure.clear();
            INFOc("Flushing Engine Config to {}", CONFIG_FILE_NAME);
            ready = false;
        }


        inline static bool initialise()
        {
            bool exists = true;

            mutex_lock config_lock(mut_config_lock);

            try {
                if (std::filesystem::exists(CONFIG_FILE_NAME) == false)
                    ready = exists = false;

                else if (ini_config_ref == nullptr)
                    ready = false;

                else if (ini_config_ref->read(ini_structure) == false)
                    ready = false;

                else if (ini_structure["About"].has("OS") == false)	// MUST have OS tag
                    ready = false;

                if (ready == false)
                {
                    static mINI::INIFile config_file(CONFIG_FILE_NAME);
                    ini_config_ref = &config_file;

                    //#ifdef WINDOWS
                    ini_structure["About"]["OS"] = "Windows";
                    //#else
                    //ini_structure["About"]["OS"] = "Other";
                    //#endif

                    if (exists == false)
                        ini_config_ref->generate(ini_structure, true);

                    ready = ini_config_ref->read(ini_structure);

                    return ready;
                }
            }
            catch (const std::exception& e)
            {
                OhSHITc("File Wizard failed to initialise our INI config wrapper thing because: {}", e.what());
            }
        }

    private:
        inline static mINI::INIFile* ini_config_ref;

        inline static mINI::INIStructure ini_structure;
        inline static bool ready = false;
        inline static std::mutex mut_config_lock;
    };
};

