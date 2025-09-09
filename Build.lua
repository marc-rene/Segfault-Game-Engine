-- premake5.lua
workspace "Segfault Game Engine"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "Segfault Editor"

   -- Workspace-wide build options for MSVC
   filter "system:windows"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

group "Utility"
	include "Misc/Logging/build_logger.lua"
	include "Misc/Networking/build_networking.lua"
	include "Misc/File IO/build_fileIO.lua"
group ""
   
include "SegFault Editor/build_editor.lua"