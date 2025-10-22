#pragma once

#include "../Resources/EngineSettings.hpp"

namespace ENGINE::Settings
{
	struct InputSettings : PrimativeSettings
	{
		bool b_enableJoyStick;
		bool b_enableHaptic;
		bool b_enableGamepad;

		bool SetDefault() override
		{
			b_enableJoyStick = false;
			b_enableHaptic = false;
			b_enableGamepad = false;

			return true;
		}

		// Change parameter from reference to value to allow switch on integral type
		bool GetValue_bool(E_Settings SettingID) override
		{
			switch (SettingID)
			{
			case INPUT_ENABLE_JOYSTICK:
				return b_enableJoyStick;
				break;
			case INPUT_ENABLE_HAPTICS:
				return b_enableHaptic;
				break;
			case INPUT_ENABLE_GAMEPAD:
				return b_enableGamepad;
				break;
			default:
				WARNc("Input settings has no idea what is");
				return false;
				break;
			}
		}
	};
};