#pragma once

#include "../Core/Log.hpp"
#include <mutex>
#include "TypeDefinitions.hpp"
#include "../Platform/File Wizard.hpp"
#include <map>


// !!! REMEMBER !!!
// !!! REMEMBER !!!
// When Adding Something here, UPDATE GetSettingsPrefix() IN EngineSettings.cpp otherwise Config will be Banjaxxed
#define CONFIG_SETTINGS_LIST    \
X(INVALID_SETTING)              \
 /* Input */                    \
X(INPUT_ENABLE_JOYSTICK)        \
X(INPUT_ENABLE_HAPTICS)         \
X(INPUT_ENABLE_GAMEPAD)         \
 /* Window */                   \
X(WINDOW_TITLE)                 \
X(WINDOW_PREFERRED_WIDTH)       \
X(WINDOW_MINIMUM_WIDTH)			\
X(WINDOW_PREFERRED_HEIGHT)      \
X(WINDOW_MINIMUM_HEIGHT)		\
X(WINDOW_IS_FULLSCREEN)		    \
X(WINDOW_IS_BORDERLESS)		    \
X(WINDOW_IS_RESIZABLE)		    \
X(WINDOW_IS_MINIMISED)		    \
X(WINDOW_IS_MAXIMISED)			\
X(WINDOW_IS_ALWAYS_ON_TOP)		\
X(VIDEO_ENABLE_VSYNC)			\


namespace ENGINE::Settings
{

	enum class E_Settings : uMint {
#define X(name) name,
		CONFIG_SETTINGS_LIST
#undef X
	};


	// --- Helper Functions ----------------------
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
	// -------------------------------------------



	class ActiveSettings
	{
	public:
		static mint*		GetSetting_mint	(E_Settings SettingID) { return &GetMintConfigVars()->at(SettingID); };
		static int*			GetSetting_int	(E_Settings SettingID) { return &GetIntConfigVars()->at(SettingID); };
		static float*		GetSetting_float(E_Settings SettingID) { return &GetFloatConfigVars()->at(SettingID); };
		static std::string* GetSetting_str	(E_Settings SettingID) { return &GetStringConfigVars()->at(SettingID); };
		
	
		inline static bool	SetSetting_mint(E_Settings SettingID, mint NewValue)		{ GetMintConfigVars()->at(SettingID) = NewValue;	return UpdateConfig(1); }
		inline static bool	SetSetting_int(E_Settings SettingID, int NewValue)			{ GetIntConfigVars()->at(SettingID) = NewValue;		return UpdateConfig(2); }
		inline static bool	SetSetting_float(E_Settings SettingID, float NewValue)		{ GetFloatConfigVars()->at(SettingID) = NewValue;	return UpdateConfig(3); }
		inline static bool	SetSetting_str(E_Settings SettingID, std::string NewValue)	{ GetStringConfigVars()->at(SettingID) = NewValue;	return UpdateConfig(4); }


		inline static std::map<E_Settings, mint>* GetMintConfigVars() { return &HotMap_mint; }
		inline static std::map<E_Settings, int>* GetIntConfigVars() { return &HotMap_int; }
		inline static std::map<E_Settings, float>* GetFloatConfigVars() { return &HotMap_float; }
		inline static std::map<E_Settings, std::string>* GetStringConfigVars() { return &HotMap_str; }

		static bool Initialise();

	private:
		static bool UpdateConfig(mint); // Update our config file based on our new hotmap
		static constexpr const std::string GetSettingsPrefix(E_Settings SettingID);

		inline static std::map<E_Settings, mint>		HotMap_mint; // Bool Settings we'll be getting alot of at Runtime
		inline static std::map<E_Settings, int>			HotMap_int;
		inline static std::map<E_Settings, float>		HotMap_float;
		inline static std::map<E_Settings, std::string> HotMap_str;
		
		inline static std::mutex			mtx_settingsMutex;
		
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