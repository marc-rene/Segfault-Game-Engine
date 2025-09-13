-- premake5.lua
workspace "Segfault Game Engine"
architecture "x64"
configurations {"Debug", "Release", "Dist"}
startproject "Segfault Editor"
defines {
    "PROJECT_NAME=\"SegFault Engine    \"", 
    "PROJECT_NAME_NOSPACE=\"SegFault_Engine\"",
    "PROKECT_DISPLAY_NAME=\"SegFault Engine\""
}

-- Workspace-wide build options for MSVC
filter "system:windows"
buildoptions {"/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus", "/utf-8"}

OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

group "Utility"
include "Misc/Logging/Build-Logger.lua"
include "Misc/Networking/Build-Networking.lua"
include "Misc/File IO/Build-File_IO.lua"
include "Misc/Audio/Build-Audio.lua"
group ""

include "SegFault Editor/build_editor.lua"

filter "configurations:Debug"
defines {"DEBUG"}
runtime "Debug"
symbols "On"
staticruntime "off"

filter "configurations:Release"
defines {"RELEASE"}
runtime "Release"
optimize "On"
symbols "On"
staticruntime "on"

filter "configurations:Dist"
defines {"DIST"}
runtime "Release"
optimize "On"
symbols "Off"
staticruntime "on"