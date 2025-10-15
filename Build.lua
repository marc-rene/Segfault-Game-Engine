-- premake5.lua
workspace "Segfault"
    architecture "x64"
    configurations {"Debug", "Release", "Distrubtion"}
    startproject "Sample Window"
    defines {"ENGINE_NAME=\"SegFault Engine\"", "ENGINE_NAME_NOSPACE=\"SegFault_Engine\"",
            "PROKECT_DISPLAY_NAME=\"SegFault Engine\""}
    cppdialect "C++23"
    staticruntime "On"

    flags {"MultiProcessorCompile"}

-- Workspace-wide build options for MSVC
filter "system:windows"
    buildoptions {"/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus", "/utf-8"}
    defines {"WINDOWS"}

filter "configurations:Debug"
    defines {"DEBUG"}
    runtime "Debug"
    symbols "On"
    staticruntime "off"

filter "configurations:Release"
    defines {"RELEASE"}
    runtime "Release"
    optimize "Speed"
    symbols "On"
    staticruntime "on"

filter "configurations:Distrubtion"
    defines {"DISTRUBTION"}
    runtime "Release"
    optimize "Speed"
    symbols "Off"
    staticruntime "on"

require "Setup/common"
OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

group "Engine"
    project "Segfault Game Engine"
    kind "StaticLib"
    cppdialect "C++23"

    -- THANK YOU CHATGPT FOR REWRITING THIS MESS
    files {
        "%{wks.location}/Audio/**.{c,cpp,h,hpp,inl,ixx,mm,hlsl}",
        "%{wks.location}/Core/**.{c,cpp,h,hpp,inl,ixx,mm,hlsl}", 
        "%{wks.location}/ECS/**.{c,cpp,h,hpp,inl,ixx,mm,hlsl}",
        "%{wks.location}/Gameplay/**.{c,cpp,h,hpp,inl,ixx,mm,hlsl}",
        "%{wks.location}/Networking/**.{c,cpp,h,hpp,inl,ixx,mm,hlsl}",
        "%{wks.location}/Platform/**.{c,cpp,h,hpp,inl,ixx,mm,hlsl}",
        "%{wks.location}/Rendering/**.{c,cpp,h,hpp,inl,ixx,mm,hlsl}",
        "%{wks.location}/Resources/**.{c,cpp,h,hpp,inl,ixx,mm,hlsl}",
        "%{wks.location}/Runtime/**.{c,cpp,h,hpp,inl,ixx,mm,hlsl}"
    }

    UseCommonIncludeDirs()
group ""

---------------------------------------
-- Auto-generate one project per example
---------------------------------------
group "Examples"
CreateExampleProjects("Examples", function(name, dir)
    project(name)
    kind "ConsoleApp"
    language "C++"

    files {
        dir .. "/**.h", 
        dir .. "/**.hpp", 
        dir .. "/**.c", 
        dir .. "/**.cpp",
        dir .. "/**.hlsl"
    }

    -- Pull in engine usage (includes + links) in one line:
    UseCommonIncludeDirs()
    UseApocalypseEngine()
end)
group ""
