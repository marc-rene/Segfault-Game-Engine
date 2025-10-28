#include "Window.hpp"

constexpr const char* WINDOW_MANAGER_NAME = "ENGINE::Platform::WindowManager";


bool ENGINE::Platform::WindowManager::Initialise()
{

    if (GetMainWindowRef() != nullptr && b_isInitialised == true)
        return true;

    else
    {
        SDL_InitFlags flags = SDL_INIT_VIDEO;
        flags |= SDL_INIT_AUDIO;
        //flags |= SDL_INIT_EVENTS;


        namespace S = ENGINE::Settings;

        auto (*GetMintSetting) = S::ActiveSettings::GetSetting_mint;

        if (*GetMintSetting(S::E_Settings::INPUT_ENABLE_JOYSTICK) == true)
        {
            TRACE(WINDOW_MANAGER_NAME, "Enabling Joystick");
            flags |= SDL_INIT_JOYSTICK;
        }

        if (*GetMintSetting(S::E_Settings::INPUT_ENABLE_HAPTICS) == true) {
            TRACE(WINDOW_MANAGER_NAME, "Enabling Haptic feedback");
            flags |= SDL_INIT_HAPTIC;
        }

        if (*GetMintSetting(S::E_Settings::INPUT_ENABLE_GAMEPAD) == true) {
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
SDL_Window* ENGINE::Platform::WindowManager::CreateMainWindow(bool p_Force)
{
    SDL_WindowFlags windowFlags = 0;
    namespace S = ENGINE::Settings;

    if (GetMainWindowRef() != nullptr) {
        WARN(WINDOW_MANAGER_NAME, "HEY! We already have a SDL Window already??? wtf?");

        if (p_Force == false) {
            return m_MainSDLWindow;
        }
    }

    auto settingsInt_ptr = S::ActiveSettings::GetIntConfigVars();
    auto settingsMint_ptr = S::ActiveSettings::GetMintConfigVars();

    if (settingsMint_ptr->at(S::E_Settings::WINDOW_IS_ALWAYS_ON_TOP))
        windowFlags |= SDL_WINDOW_ALWAYS_ON_TOP;

    if (settingsMint_ptr->at(S::E_Settings::WINDOW_IS_BORDERLESS))
        windowFlags |= SDL_WINDOW_BORDERLESS;

    if (settingsMint_ptr->at(S::E_Settings::WINDOW_IS_FULLSCREEN))
        windowFlags |= SDL_WINDOW_FULLSCREEN;

    if (settingsMint_ptr->at(S::E_Settings::WINDOW_IS_MAXIMISED))
        windowFlags |= SDL_WINDOW_MAXIMIZED;

    else if (settingsMint_ptr->at(S::E_Settings::WINDOW_IS_MINIMISED))
        windowFlags |= SDL_WINDOW_MINIMIZED;

    if (settingsMint_ptr->at(S::E_Settings::WINDOW_IS_RESIZABLE))
        windowFlags |= SDL_WINDOW_RESIZABLE;


    m_MainSDLWindow = SDL_CreateWindow(
        S::ActiveSettings::GetSetting_str(S::E_Settings::WINDOW_TITLE)->c_str(),
        settingsInt_ptr->at(S::E_Settings::WINDOW_PREFERRED_WIDTH),
        settingsInt_ptr->at(S::E_Settings::WINDOW_PREFERRED_HEIGHT),
        windowFlags
    );


    if (m_MainSDLWindow == nullptr) {
        OhSHIT(WINDOW_MANAGER_NAME, "SDL_CreateWindow FAILED! SDL SAYS: {}", SDL_GetError());

        return nullptr;
    }


    if (SDL_SetWindowMinimumSize(m_MainSDLWindow,
        settingsInt_ptr->at(S::E_Settings::WINDOW_MINIMUM_WIDTH),
        settingsInt_ptr->at(S::E_Settings::WINDOW_MINIMUM_HEIGHT)) == false)
    {
        WARN(WINDOW_MANAGER_NAME, "SDL_SetWindowMinimumSize Failed... HOW??? IT'S JUST SAYING \
            \"HEY! I CAN'T BE SMALLER THAN {} x {}... WTF??? WORDS CANNOT DESCRIBE MY CONFUSION!\"",
            settingsInt_ptr->at(S::E_Settings::WINDOW_MINIMUM_WIDTH), settingsInt_ptr->at(S::E_Settings::WINDOW_MINIMUM_HEIGHT));
    }
    return m_MainSDLWindow;
}



SDL_Window* ENGINE::Platform::WindowManager::GetMainWindowRef()
{
    return ENGINE::Platform::WindowManager::m_MainSDLWindow;
}



mint ENGINE::Platform::WindowManager::PollEvents()
{
    if (GetInstance()->b_isInitialised == false)
        ERROR(WINDOW_MANAGER_NAME, "How is a unInitialised Window supposed to get any events?");

    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            return 0;
        }

        if (event.type == SDL_EVENT_KEY_DOWN)
        {
            TRACE(WINDOW_MANAGER_NAME, "OI BLYAT KEY IS DOWN!!!");
        }

        if (event.type == SDL_EVENT_MOUSE_MOTION)
        {
            //INFO(WINDOW_MANAGER_NAME, "OI BLYAT MOUSE GO BRRRRR!!!");
        }
        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
        {
            auto mouse_x = &ENGINE::Platform::WindowManager::GetInstance()->Mouse_pos.x;
            auto mouse_y = &ENGINE::Platform::WindowManager::GetInstance()->Mouse_pos.y;

            SDL_GetMouseState(mouse_x, mouse_y);
            TRACE(WINDOW_MANAGER_NAME, "OI BLYAT MOUSE IS CLICKING DOWN AT X: {}, Y: {}",
                ENGINE::Platform::WindowManager::GetInstance()->Mouse_pos.x,
                ENGINE::Platform::WindowManager::GetInstance()->Mouse_pos.y);

        }
        if (event.type == SDL_EVENT_WINDOW_MOUSE_ENTER)
        {
            //INFO(WINDOW_MANAGER_NAME, "OI BLYAT MOUSE GO BRRRRR!!!");
        }
        if (event.type == SDL_EVENT_WINDOW_MOUSE_LEAVE)
        {
            //INFO(WINDOW_MANAGER_NAME, "OI BLYAT MOUSE GO BRRRRR!!!");
        }
        if (event.type == SDL_EVENT_WINDOW_FOCUS_GAINED)
        {
            //INFO(WINDOW_MANAGER_NAME, "OI BLYAT MOUSE GO BRRRRR!!!");
        }

        else
        {
            //TRACE(WINDOW_MANAGER_NAME, "OI EVENT WAS CALLED: {}", event.type);
        }
    }

    return 1;
}


