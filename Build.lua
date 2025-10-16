-- premake5.lua
workspace "Segfault"
    architecture "x64"
    configurations {"Debug", "Release", "Distrubtion"}
    -- For SDL because it only has debug and release:
    configmap {
        ["Distrubtion"] = "Release",
    }
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

-- ChatGPT Generated... seems legit
filter "files:**.hlsl"
    buildmessage "DXC %{file.name}"
    -- Map shader type to profile via filename or custom rule; example assumes compute:
    buildcommands {
        'dxc -nologo -E main -T cs_6_6 -O3 -Zpr -Fo "%{cfg.targetdir}/%{file.basename}.cso" "%{file.relpath}"'
    }
    buildoutputs { "%{cfg.targetdir}/%{file.basename}.cso" }
filter {}

-- TODO: FIX THIS
OutputDir = "%{cfg.architecture}/%{cfg.buildcfg}"
SetOutputDirs()


----------------------------------------
-- Thirdparty (SDL3 + ImGui)
----------------------------------------
group "ThirdParty"
    -- SDL3 (no vulkan, only DirectX 12)
    externalproject "SDL"
       location "%{wks.location}/Platform/SDL/VisualC/SDL"
        
        -- HEY THERE! IF YOU'RE HAVING SDL PROJECT ISSUES, CHECK SDL.vcxproj IN VISUALC FOLDER AND CHECK THAT <ProjectGuid> MATCHES
        uuid "81CE8DAF-EBB2-4761-8E45-B71ABCCA8C68" 
        kind "StaticLib"
        language "C"
        staticruntime "On"

        SetOutputDirs()
        
    -- ImGui (only DirectX 12 + SDL3)
    project "imgui_sdl3_dx12"
        kind "StaticLib"
        language "C++"
        cppdialect "C++20"
        files {
            "%{wks.location}/Rendering/Imgui/imconfig.h",
            "%{wks.location}/Rendering/Imgui/imgui.h",
            "%{wks.location}/Rendering/Imgui/imgui.cpp",
            "%{wks.location}/Rendering/Imgui/imgui_draw.cpp",
            "%{wks.location}/Rendering/Imgui/imgui_tables.cpp",
            "%{wks.location}/Rendering/Imgui/imgui_widgets.cpp",
            
            -- backends
            "%{wks.location}/Rendering/Imgui/backends/imgui_impl_dx12.cpp",
            "%{wks.location}/Rendering/Imgui/backends/imgui_impl_sdl3.cpp",
        }
        
        includedirs {
            "%{wks.location}/Rendering/Imgui",
            "%{wks.location}/Rendering/Imgui/backends",
            "%{wks.location}/Platform/SDL/include",
        }
        
        excludes {
            "%{wks.location}/Rendering/Imgui/backends/imgui_impl_vulkan.*",
            "%{wks.location}/Rendering/Imgui/backends/imgui_impl_glfw.*",
            "%{wks.location}/Rendering/Imgui/backends/imgui_impl_sdlrenderer.*",
            "%{wks.location}/Rendering/Imgui/backends/imgui_impl_sdl2.*",
            "%{wks.location}/Rendering/Imgui/backends/imgui_impl_win32.*",
        }

        SetOutputDirs()

        dependson { "SDL" }

group ""
        
        
       
----------------------------------------
-- Segfault Engine
----------------------------------------
        
project "Segfault Game Engine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++23"
    
    targetname "SegfaultGameEngine"
    SetOutputDirs()

    -- THANK YOU CHATGPT FOR REWRITING THIS MESS
    files {
        "%{wks.location}/**.c",
        "%{wks.location}/**.cpp",
        "%{wks.location}/**.h",
        "%{wks.location}/**.hpp",
        "%{wks.location}/**.inl",
        "%{wks.location}/**.ixx",
        "%{wks.location}/**.mm",
        "%{wks.location}/**.hlsl",

        --"%{wks.location}/Core/**.{c,cpp,h,hpp,inl,ixx,mm,hlsl}",
        --"%{wks.location}/ECS/**.{c,cpp,h,hpp,inl,ixx,mm,hlsl}",
        --"%{wks.location}/Gameplay/**.{c,cpp,h,hpp,inl,ixx,mm,hlsl}",
        --"%{wks.location}/Networking/**.{c,cpp,h,hpp,inl,ixx,mm,hlsl}",
        --"%{wks.location}/Platform/**.{c,cpp,h,hpp,inl,ixx,mm,hlsl}",
        --"%{wks.location}/Rendering/**.{c,cpp,h,hpp,inl,ixx,mm,hlsl}",
        --"%{wks.location}/Resources/**.{c,cpp,h,hpp,inl,ixx,mm,hlsl}",
        --"%{wks.location}/Runtime/**.{c,cpp,h,hpp,inl,ixx,mm,hlsl}",
    }
        
    excludes {
        "%{wks.location}/Examples/**",
        "%{wks.location}/Setup/**",
        "%{wks.location}/Platform/SDL/**", -- third-party is built in its own projects
        "%{wks.location}/Core/Spdlog/**",
        "%{wks.location}/Core/Math/**",
        "%{wks.location}/ECS/entt.hpp",
        "%{wks.location}/Platform/EnkiTS/**",
        "%{wks.location}/Platform/SDL/**/vulkan/**",
        "%{wks.location}/Rendering/Imgui/backends/imgui_impl_vulkan.*",
        "%{wks.location}/Rendering/Imgui/backends/imgui_impl_vulkan.*",
        "%{wks.location}/Rendering/DirectX 12/Memory Allocator/**",
        "%{wks.location}/Rendering/DirectX 12/Toolkit/**",
        "%{wks.location}/Gameplay/Navigation/Recast/**",
        "%{wks.location}/Networking/Game Networking Sockets/**",
    }
    
    UseCommonIncludeDirs()
    
    links { SDL3_Path, 
    "imgui_sdl3_dx12" }
    dependson { "SDL" }





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
