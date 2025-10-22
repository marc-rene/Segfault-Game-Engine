#include "Window.hpp"



bool ENGINE::Platform::WindowManager::Initialise()
{
	if (GetMainWindowRef() != nullptr && b_isInitialised == true)
		return true;

	else
	{
		SDL_InitFlags flags = SDL_INIT_VIDEO;
		flags |= SDL_INIT_AUDIO;
		flags |= SDL_INIT_EVENTS;


		if (ENGINE::Settings::ActiveSettings::GetInputSettings()->GetValue_bool(ENGINE::Settings::E_Settings::INPUT_ENABLE_JOYSTICK) == true) {
			TRACE(WINDOW_MANAGER_NAME, "Enabling Joystick");
			flags |= SDL_INIT_JOYSTICK;
		}

		if (ENGINE::Settings::ActiveSettings::GetInputSettings()->GetValue_bool(ENGINE::Settings::E_Settings::INPUT_ENABLE_HAPTICS) == true) {
			TRACE(WINDOW_MANAGER_NAME, "Enabling Haptic feedback");
			flags |= SDL_INIT_HAPTIC;
		}

		if (ENGINE::Settings::ActiveSettings::GetInputSettings()->GetValue_bool(ENGINE::Settings::E_Settings::INPUT_ENABLE_GAMEPAD) == true) {
			TRACE(WINDOW_MANAGER_NAME, "Enabling Gamepad");
			flags |= SDL_INIT_GAMEPAD;
		}

		b_isInitialised = SDL_Init(flags);
	}

	if (b_isInitialised == true) {
		INFO(WINDOW_MANAGER_NAME, "Initialise() was a great success! SDL IS GOOD TO GO!");
	}

	else {
		OhSHIT(WINDOW_MANAGER_NAME, "SDL_Init FAILED! SDL SAYS: {}", SDL_GetError());
	}

	return b_isInitialised;
}



/// @brief Try Create a window, wrapping around SDL_CreateWindow
/// @param force Create the window anyway and overwrite!
SDL_Window* ENGINE::Platform::WindowManager::CreateMainWindow(ENGINE::Settings::WindowSettings* p_windowProperties, bool force)
{
	if (GetMainWindowRef() != nullptr) {
		WARN(WINDOW_MANAGER_NAME, "HEY! We already have a SDL Window already??? wtf?");

		if (force == false) {
			return m_MainSDLWindow;
		}
	}
	
	m_MainSDLWindow = SDL_CreateWindow(
		p_windowProperties->DefaultWindowTitle.c_str(),
		p_windowProperties->int_preferredWindowWidth,
		p_windowProperties->int_preferredWindowHeight,
		p_windowProperties->SDLFlag_windowProperties
	);

	if (m_MainSDLWindow == nullptr) {
		OhSHIT(WINDOW_MANAGER_NAME, "SDL_CreateWindow FAILED! SDL SAYS: {}", SDL_GetError());
		
		return nullptr;
	}

	return m_MainSDLWindow;
}



SDL_Window* ENGINE::Platform::WindowManager::GetMainWindowRef()
{
	return m_MainSDLWindow;
}



ENGINE::Platform::WindowManager::WindowManager()
{
	Initialise();
}
