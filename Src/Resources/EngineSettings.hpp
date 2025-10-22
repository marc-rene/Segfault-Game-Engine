#pragma once

#include "../Core/Log.hpp"
#include <mutex>
#include "TypeDefinitions.hpp"
#include "../Platform/File Wizard.hpp"


#define CONFIG_SETTINGS_LIST    \
X(INVALID_SETTING)              \
 /* Input */                    \
X(INPUT_ENABLE_JOYSTICK)        \
X(INPUT_ENABLE_HAPTICS)         \
X(INPUT_ENABLE_GAMEPAD)         \
 /* Window */                   \
X(WINDOW_TITLE)                 \
X(WINDOW_PREFERRED_WIDTH)       \
X(WINDOW_PREFERRED_HEIGHT)      \
X(WINDOW_IS_FULLSCREEN)		    \
X(WINDOW_IS_BORDERLESS)		    \
X(WINDOW_IS_RESIZABLE)		    \
X(WINDOW_IS_MINIMISED)		    \
X(WINDOW_IS_MAXIMISED)      \
X(WINDOW_IS_ALWAYS_ON_TOP)  \


namespace ENGINE::Settings
{

	enum class E_Settings : uMint {
#define X(name) name,
		CONFIG_SETTINGS_LIST
#undef X
	};

	// Helper Functions

	constexpr std::string ToString(E_Settings SettingID) {
	
		switch (SettingID)
		{
#define X(name) case E_Settings::name: return #name;
			CONFIG_SETTINGS_LIST
#undef X

		default:
			return "INVALID_SETTING";
		}
	}
	constexpr std::string_view	ToStringView(E_Settings);


	// Any settings which are just ints, bools, etcs... will go here
	struct PrimativeSettings
	{
		virtual bool SetDefault()
		{
			WARNc("Someone is trying to PrimativeSettings::SetDefault() using the default method... nothing's been defined... wtf?");
			return false;
		}

		virtual bool SetSettingsFromConfig()
		{
			WARNc("Someone is trying to PrimativeSettings::SetSettingsFromConfig() using the default method... nothing's been defined... wtf?");
			return false;
		}

		PrimativeSettings()
		{
			if (SetSettingsFromConfig() == false)
				SetDefault();
		}
	};


	/// @brief Singleton - Only one ACTIVE set of global settings
	class ActiveSettings
	{
	public:

		// DO NOT USE COPY... ActiveSettings is a SINGLETON
		ActiveSettings(const ActiveSettings& obj_to_ignore) = delete;

		static ActiveSettings* GetActiveSettings() {
			if (m_activeSettings_ref == nullptr) {
				std::lock_guard<std::mutex> lock(m_settingsMtx);

				if (m_activeSettings_ref == nullptr) {
					m_activeSettings_ref = new ActiveSettings();
				}
			}
			return m_activeSettings_ref;
		}


		inline static std::map<E_Settings, bool>* GetBoolConfigVars() { return &HotMap_bool; }
		inline static std::map<E_Settings, int>* GetIntConfigVars() { return &HotMap_int; }
		inline static std::map<E_Settings, std::string>* GetStringConfigVars() { return &HotMap_str; }




	private:
		inline static std::map<E_Settings, bool> HotMap_bool; // Bool Settings we'll be getting alot of at Runtime
		inline static std::map<E_Settings, int> HotMap_int;
		inline static std::map<E_Settings, std::string> HotMap_str;
		inline static std::mutex m_settingsMtx;
		inline static ActiveSettings* m_activeSettings_ref;

		ActiveSettings()
		{
			namespace fw = ENGINE::Platform::FileIO;

			HotMap_bool = std::map<E_Settings, bool>();
			HotMap_bool[E_Settings::INPUT_ENABLE_JOYSTICK] = fw::Config::GetSetting_bool("INPUT", ToString(E_Settings::INPUT_ENABLE_JOYSTICK));
			HotMap_bool[E_Settings::INPUT_ENABLE_HAPTICS] = fw::Config::GetSetting_bool("INPUT", ToString(E_Settings::INPUT_ENABLE_HAPTICS));
			HotMap_bool[E_Settings::INPUT_ENABLE_GAMEPAD] = fw::Config::GetSetting_bool("INPUT", ToString(E_Settings::INPUT_ENABLE_GAMEPAD));




			INFO("ENGINE::Settings", "TODO: Add default settings constructor and 'getters' from disk.. for now Ca marche :)");
		}

	};

}



// TODO : Replace with other hash because I dont trust HashGPT (fnv1a hash)
constexpr uint32_t GPT_Hash(std::string_view str)
{
	uint32_t hash = 0x811C9DC5u;
	for (char c : str)
		hash = (hash ^ static_cast<uint32_t>(c)) * 0x01000193u;
	return hash;
}



constexpr std::string_view ToStringView(ENGINE::Settings::E_Settings SettingID)
{
	using namespace ENGINE::Settings;

	switch (SettingID)
	{
#define X(name) case E_Settings::name: return #name;
		CONFIG_SETTINGS_LIST
#undef X

	default:
		return "INVALID_SETTING";
	}
}



constexpr std::string ToString(ENGINE::Settings::E_Settings SettingID) {
	using namespace ENGINE::Settings;

	switch (SettingID)
	{
#define X(name) case E_Settings::name: return #name;
		CONFIG_SETTINGS_LIST
#undef X

	default:
		return "INVALID_SETTING";
	}
}



constexpr ENGINE::Settings::E_Settings FromString(std::string_view str) {
	using namespace ENGINE::Settings;
	switch (GPT_Hash(str))
	{
#define X(name) case GPT_Hash(#name): return E_Settings::name;
		CONFIG_SETTINGS_LIST
#undef X
	default: return E_Settings::INVALID_SETTING;
	}
}