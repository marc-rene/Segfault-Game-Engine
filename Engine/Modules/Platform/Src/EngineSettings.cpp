#include "EngineSettings.hpp"



bool ENGINE::Settings::ActiveSettings::Initialise()
{
    namespace fw = ENGINE::Platform::FileIO;

    std::lock_guard<std::mutex> lock(mtx_settingsMutex);

    fw::Config::initialise();

    //Input Settings First
    E_Settings NEXT = E_Settings::INPUT_ENABLE_JOYSTICK;
    HotMap_mint[NEXT] = fw::Config::GetSetting_mint(GetSettingsPrefix(NEXT), ToString(NEXT), false);

    NEXT = E_Settings::INPUT_ENABLE_HAPTICS;	HotMap_mint[NEXT] = fw::Config::GetSetting_mint(GetSettingsPrefix(NEXT), ToString(NEXT), false);
    NEXT = E_Settings::INPUT_ENABLE_GAMEPAD;	HotMap_mint[NEXT] = fw::Config::GetSetting_mint(GetSettingsPrefix(NEXT), ToString(NEXT), false);

    //Window Settings
    NEXT = E_Settings::WINDOW_TITLE;
    HotMap_str[NEXT] = fw::Config::GetSetting_str(GetSettingsPrefix(NEXT), ToString(NEXT), "TODO: Dont have a terrible default.. Change Me in EngineConfig.ini");

    NEXT = E_Settings::WINDOW_PREFERRED_WIDTH;	HotMap_int[NEXT] = fw::Config::GetSetting_int(GetSettingsPrefix(NEXT), ToString(NEXT), 1920);
    NEXT = E_Settings::WINDOW_MINIMUM_WIDTH;	HotMap_int[NEXT] = fw::Config::GetSetting_int(GetSettingsPrefix(NEXT), ToString(NEXT), 800);
    NEXT = E_Settings::WINDOW_PREFERRED_HEIGHT;	HotMap_int[NEXT] = fw::Config::GetSetting_int(GetSettingsPrefix(NEXT), ToString(NEXT), 1080);
    NEXT = E_Settings::WINDOW_MINIMUM_HEIGHT;	HotMap_int[NEXT] = fw::Config::GetSetting_int(GetSettingsPrefix(NEXT), ToString(NEXT), 600);
    NEXT = E_Settings::WINDOW_IS_FULLSCREEN;	HotMap_mint[NEXT] = fw::Config::GetSetting_mint(GetSettingsPrefix(NEXT), ToString(NEXT), false);
    NEXT = E_Settings::WINDOW_IS_BORDERLESS;	HotMap_mint[NEXT] = fw::Config::GetSetting_mint(GetSettingsPrefix(NEXT), ToString(NEXT), false);
    NEXT = E_Settings::WINDOW_IS_RESIZABLE;		HotMap_mint[NEXT] = fw::Config::GetSetting_mint(GetSettingsPrefix(NEXT), ToString(NEXT), true);
    NEXT = E_Settings::WINDOW_IS_MINIMISED;		HotMap_mint[NEXT] = fw::Config::GetSetting_mint(GetSettingsPrefix(NEXT), ToString(NEXT), false);
    NEXT = E_Settings::WINDOW_IS_MAXIMISED;		HotMap_mint[NEXT] = fw::Config::GetSetting_mint(GetSettingsPrefix(NEXT), ToString(NEXT), false);
    NEXT = E_Settings::WINDOW_IS_ALWAYS_ON_TOP;	HotMap_mint[NEXT] = fw::Config::GetSetting_mint(GetSettingsPrefix(NEXT), ToString(NEXT), false);
    NEXT = E_Settings::VIDEO_ENABLE_VSYNC;		HotMap_mint[NEXT] = fw::Config::GetSetting_mint(GetSettingsPrefix(NEXT), ToString(NEXT), true);

    return true;
}


/// @brief Update our enviornment vars
/// @param Which_is_dirty: 1:Bools/Mints  2:Int vars  3:Float Vars  4:String Vars  0:ALL
bool ENGINE::Settings::ActiveSettings::UpdateConfig(mint Which_is_dirty = 0)
{
    mtx_settingsMutex.lock();
    switch (Which_is_dirty)
    {
    case 1:
        for (const auto& pair : *GetMintConfigVars()) // Do our bools / chars first
            ENGINE::Platform::FileIO::Config::SetSetting(GetSettingsPrefix(pair.first), ToString(pair.first), std::to_string(pair.second));
        break;
    case 2:
        for (const auto& pair : *GetIntConfigVars()) // ints 
            ENGINE::Platform::FileIO::Config::SetSetting(GetSettingsPrefix(pair.first), ToString(pair.first), std::to_string(pair.second));
        break;
    case 3:
        for (const auto& pair : *GetFloatConfigVars()) // floats
            ENGINE::Platform::FileIO::Config::SetSetting(GetSettingsPrefix(pair.first), ToString(pair.first), std::to_string(pair.second));
        break;
    case 4:
        for (const auto& pair : *GetStringConfigVars()) // strings
            ENGINE::Platform::FileIO::Config::SetSetting(GetSettingsPrefix(pair.first), ToString(pair.first), pair.second);
        break;

    case 0:
    default:
        mtx_settingsMutex.unlock();
        ENGINE::Settings::ActiveSettings::UpdateConfig(1);
        ENGINE::Settings::ActiveSettings::UpdateConfig(2);
        ENGINE::Settings::ActiveSettings::UpdateConfig(3);
        ENGINE::Settings::ActiveSettings::UpdateConfig(4);
        break;
    }
    mtx_settingsMutex.unlock();
    return true;
}



constexpr const std::string ENGINE::Settings::ActiveSettings::GetSettingsPrefix(E_Settings SettingID)
{
    switch (SettingID)
    {
    case E_Settings::INVALID_SETTING:
    case E_Settings::INPUT_ENABLE_JOYSTICK:
    case E_Settings::INPUT_ENABLE_HAPTICS:
    case E_Settings::INPUT_ENABLE_GAMEPAD:
        return "INPUT";
        break;
    case E_Settings::WINDOW_TITLE:
    case E_Settings::WINDOW_PREFERRED_WIDTH:
    case E_Settings::WINDOW_MINIMUM_WIDTH:
    case E_Settings::WINDOW_PREFERRED_HEIGHT:
    case E_Settings::WINDOW_MINIMUM_HEIGHT:
    case E_Settings::WINDOW_IS_FULLSCREEN:
    case E_Settings::WINDOW_IS_BORDERLESS:
    case E_Settings::WINDOW_IS_RESIZABLE:
    case E_Settings::WINDOW_IS_MINIMISED:
    case E_Settings::WINDOW_IS_MAXIMISED:
    case E_Settings::WINDOW_IS_ALWAYS_ON_TOP:
        return "WINDOW";

    case E_Settings::VIDEO_ENABLE_VSYNC:
        return "VIDEO";


    default:
        return "MISC";
    }
}

