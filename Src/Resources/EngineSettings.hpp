#pragma once

#include "../Core/Log.hpp"
#include <mutex>
#include "TypeDefinitions.hpp"

#define ENUM_SETTINGS_TYPE uMint

namespace ENGINE::Settings
{
		
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	enum E_Settings : ENUM_SETTINGS_TYPE
	{
		INVALID_SETTING,
		// - - - - - - - - - - - - - - - - - - 
		// Input Settings
		INPUT_ENABLE_JOYSTICK,
		INPUT_ENABLE_HAPTICS,
		INPUT_ENABLE_GAMEPAD,
		// - - - - - - - - - - - - - - - - - - 


		// - - - - - - - - - - - - - - - - - - 
		// Window Settigns
		WINDOW_TITLE,
		WINDOW_PREFERRED_WIDTH,	// What window width will we try launch at?
		WINDOW_PREFERRED_HEIGHT, 
		WINDOW_IS_FULLSCREEN,
		WINDOW_IS_BORDERLESS,
		WINDOW_IS_RESIZABLE,
		WINDOW_IS_MINIMISED,
		WINDOW_IS_MAXIMISED,
		WINDOW_IS_ALWAYS_ON_TOP,
	};
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	


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

		virtual bool GetValue_bool(E_Settings SettingID)
		{
			WARNc("Someone is trying to PrimativeSettings::GetValue() [bool version] using the default method... nothing's been defined... wtf?");
			return false;
		}
		
		// TODO: this is terrible, what if there's a int8 that we want...
		virtual uFatty* GetValue_int(E_Settings SettingID)
		{
			WARNc("Someone is trying to PrimativeSettings::GetValue() [int version] using the default method... nothing's been defined... wtf?");
			return nullptr;
		}

		virtual std::string* GetValue_str(E_Settings SettingID)
		{
			WARNc("Someone is trying to PrimativeSettings::GetValue() [string version] using the default method... nothing's been defined... wtf?");
			return nullptr;
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

		static PrimativeSettings* GetInputSettings()
		{
			return &GetActiveSettings()->m_inputSettings;
		}

	private:
		PrimativeSettings m_inputSettings;
		static std::mutex m_settingsMtx;
		static ActiveSettings* m_activeSettings_ref;

		ActiveSettings()
		{
			INFO("ENGINE::Settings", "TODO: Add default settings constructor and 'getters' from disk.. for now Ca marche :)");
		}

	};

}