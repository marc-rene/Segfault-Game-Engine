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
    buildoptions {
        "/EHsc", 
        "/Zc:preprocessor", 
        "/Zc:__cplusplus", 
        "/utf-8", 
        "/O2", 
        "/Oi", 
        "/Ot", 
        "/GL", 
        "/fp:fast", 
        "/GS-", 
        "/arch:AVX2"
    }
    linkoptions { "/LTCG", "/LTO" }
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
    floatingpoint "Fast"
    symbols "On"
    staticruntime "on"
    flags { "linktimeoptimization", "NoIncrementalLink" }

filter "configurations:Distrubtion"
    defines {"DISTRUBTION"}
    runtime "Release"
    optimize "Speed"
    floatingpoint "Fast"
    symbols "Off"
    staticruntime "on"
    flags { "linktimeoptimization", "NoIncrementalLink" }



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
       location "%{wks.location}/ThirdParty/SDL/VisualC/SDL"
        
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
            "%{wks.location}/ThirdParty/ImGui/*.h",
            "%{wks.location}/ThirdParty/ImGui/*.hpp",
            "%{wks.location}/ThirdParty/ImGui/*.c",
            "%{wks.location}/ThirdParty/ImGui/*.cpp",
            "%{wks.location}/Rendering/Imgui/backends/imgui_impl_dx11.*",   -- JUST to be sure            
            "%{wks.location}/Rendering/Imgui/backends/imgui_impl_dx12.*",   -- JUST to be sure            
            "%{wks.location}/Rendering/Imgui/backends/imgui_impl_sdl3.*",   -- JUST to be sure
        }
        
        includedirs {
            "%{wks.location}/ThirdParty/ImGui",
            "%{wks.location}/ThirdParty/SDL/include",
        }
        
        removefiles  {
            "%{wks.location}/Rendering/Imgui/examples/**",
            
            "!%{wks.location}/Rendering/Imgui/backends/imgui_impl_dx11.*",
            "!%{wks.location}/Rendering/Imgui/backends/imgui_impl_dx12.*",
            "!%{wks.location}/Rendering/Imgui/backends/imgui_impl_sdl3.*",
            "%{wks.location}/Rendering/Imgui/backends/**",
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

    files {
        "%{wks.location}/Src/**.c",
        "%{wks.location}/Src/**.cpp",
        "%{wks.location}/Src/**.h",
        "%{wks.location}/Src/**.hpp",
        "%{wks.location}/Src/**.inl",
        "%{wks.location}/Src/**.ixx",
        "%{wks.location}/Src/**.mm",
        "%{wks.location}/Src/**.hlsl",
    }
        
    removefiles  {

    }
    
    UseCommonIncludeDirs()
    
    links { "SDL", "imgui_sdl3_dx12" }
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
