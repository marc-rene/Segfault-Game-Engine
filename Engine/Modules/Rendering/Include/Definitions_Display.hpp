/* This header file is meant to house the typedef's enums, and various other settings that will be needed for our 
 * Davinci::Display struct
 * 
 * Much of this is based off SDL_video.h so that #includes can be easier for later as we won't need a hundred different
 * #includes
*/

#pragma once
#include "Definitions_Global.hpp"


using DaVinci_WindowFlags = uInt_16; // 16 Possible Flags for our window init

#define DAVINCI_WINDOW__FULLSCREEN              BIT(0)      /**< window is in fullscreen mode                                                                               */

#define DAVINCI_WINDOW__OCCLUDED                BIT(1)      /**< window is fully blocked from view (Something else is fullscreen)                                           */

#define DAVINCI_WINDOW__BORDERLESS              BIT(2)      /**< no window decoration                                                                                       */

#define DAVINCI_WINDOW__RESIZABLE               BIT(3)      /**< window can be resized                                                                                      */

#define DAVINCI_WINDOW__MINIMIZED               BIT(4)      /**< window is minimized                                                                                        */

#define DAVINCI_WINDOW__MAXIMIZED               BIT(5)      /**< window is maximized                                                                                        */

#define DAVINCI_WINDOW__INPUT_FOCUS             BIT(6)      /**< window has input focus                                                                                     */

#define DAVINCI_WINDOW__MOUSE_FOCUS             BIT(7)      /**< window has mouse focus "cursor is currently over the window"                                               */

#define DAVINCI_WINDOW__MOUSE_GRABBED           BIT(8)      /**< window has grabbed mouse input (unrelated to MOUSE_CAPTURE) "confine the mouse cursor to your window"      */

#define DAVINCI_WINDOW__MOUSE_CAPTURE           BIT(9)      /**< window has mouse captured (unrelated to MOUSE_GRABBED) "keep receiving mouse events outside the window"    */

#define DAVINCI_WINDOW__ALWAYS_ON_TOP           BIT(10)     /**< window should always be above others                                                                       */

#define DAVINCI_WINDOW__KEYBOARD_GRABBED        BIT(11)     /**< window has grabbed keyboard input                                                                          */

#define DAVINCI_WINDOW__VULKAN                  BIT(12)     /**< window usable for Vulkan surface                                                                           */

#define DAVINCI_WINDOW__TRANSPARENT             BIT(13)     /**< window with transparent buffer                                                                             */

#define DAVINCI_WINDOW__NOT_FOCUSABLE           BIT(14)     /**< window should not be focusable                                                                             */



// Width == array[0] and Height == array[1]
// ALWAYS be width * height
using DaVinci_WindowDimensions = uInt_32[2];


// Stand-in for SDL_EventType because we don't want SDL include hell
enum E_DaVinci_WindowEventType : uMint
{
    NOTHING = 0,
    QUIT,
    WINDOW_RESIZED,
    WINDOW_MOVED_TO_NEW_DISPLAY,
    WINDOW_IS_HIDDEN,
    WINDOW_IS_NO_LONGER_HIDDEN,
    LOW_MEMORY,
    ENTERED_BACKGROUND,
    ENTERED_FOREGROUND,
    DARK_MODE_THEME_CHANGED,

    DISPLAY_ADDED,
    DISPLAY_REMOVED,
    DISPLAY_RENDER_DEVICE_CHANGED,
    DISPLAY_HDR_STATUS_CHANGED,

    // --- Keyboard Events ---//
    KEY_DOWN,
    KEY_UP,
    KEYMAP_CHANGED,
    // -----------------------//

    // ---  Mouse Events   ---//
    MOUSE_ENTERED_WINDOW,
    MOUSE_HAS_MOVED,
    MOUSE_BUTTON_DOWN,
    MOUSE_BUTTON_UP,
    MOUSE_WHEEL_MOVED,
    // -----------------------//

    // --- Gamepad Events --- //
    GAMEPAD_AXIS_MOTION,
    GAMEPAD_BUTTON_DOWN,
    GAMEPAD_BUTTON_UP,
    GAMEPAD_ADDED,
    GAMEPAD_REMOVED,
    GAMEPAD_REMAPPED,
    GAMEPAD_TOUCHPAD_DOWN,
    GAMEPAD_TOUCHPAD_MOTION,
    GAMEPAD_TOUCHPAD_UP,
    // -----------------------//

    // ---  Audio Events   ---//
    AUDIO_DEVICE_ADDED,
    AUDIO_DEVICE_REMOVED,
    AUDIO_DEVICE_FORMAT_CHANGED,
    // -----------------------//
};


// What RHI are we using? D3D-11? D3D-12? Vulkan?
enum E_DaVinci_RenderAPI : uMint
{
#ifdef DIRECTX_11_SUPPORT
    DirectX_11,
#endif

#ifdef DIRECTX_12_SUPPORT
    DirectX_12,
#endif
    
#ifdef VULKAN_SUPPORT
    Vulkan,
#endif
    
#ifdef WEBGPU_SUPPORT
    Web_GPU,
#endif
};
