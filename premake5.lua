include "./Setup/Common.lua"

workspace "Segfault"
	configurations { "Debug", "Release", "Dist" }
    platforms { "Win32", "x64" }
    
    filter "platforms:Win32"
    architecture "x86"

    filter "platforms:x64"
    architecture "x64"

    filter {}

    startproject "Sample Window"

    -- For SDL because it only has debug and release:
    configmap {
        ["Dist"] = "Release",
    }

	language "C++"
	cppdialect "C++23"
	staticruntime "Off"

	flags { "MultiProcessorCompile" }

	defines {
		"SPDLOG_USE_STD_FORMAT",
        "ENGINE_NAME=\"SegFault Engine\"", 
        "ENGINE_NAME_NOSPACE=\"SegFault_Engine\"",
        "PROKECT_DISPLAY_NAME=\"SegFault Engine\""
	}

    filter "action:vs*"
        linkoptions { "/ignore:4099" }  -- Disable no PDB found warning
        disablewarnings { "4068" }      -- Disable "Unknown #pragma mark warning"


    -- Configurations ----------------------------------------
    --  Debug Config
	filter "configurations:Debug"
        defines {"ENGINE_DEBUG_MODE"}
        runtime "Debug"
        optimize "Debug"
        symbols "On"
    
    --  Release Config
	filter "configurations:Release"
        defines {"ENGINE_RELEASE_MODE"}    
        runtime "Release"
        optimize "Speed"
        floatingpoint "Fast"
        symbols "Default"
        staticruntime "on"
        buildoptions { 
            "/O2", 
            "/Oi", 
            "/Ot", 
            "/GL", 
            "/fp:fast", 
            "/GS-", 
            "/arch:AVX2"
        }
    
    --  Distribution Config
	filter "configurations:Dist"
        defines {"ENGINE_RELEASE_MODE", "ENGINE_DISTRIBUTION_MODE"}
        runtime "Release"
        optimize "Speed"
        floatingpoint "Fast"
        symbols "Off"
        staticruntime "on"
        buildoptions {
            "/O2", 
            "/Oi", 
            "/Ot", 
            "/GL", 
            "/fp:fast", 
            "/GS-", 
            "/arch:AVX2"
        }
    ----------------------------------------------------------
    
	filter "system:windows"
    	defines { "PLATFORM_IS_WINDOWS" }

	filter "system:linux"
		defines { "PLATFORM_IS_LINUX" }
    

    filter "files:**.hlsl"
        buildmessage "DXC %{file.name}"
        -- Map shader type to profile via filename or custom rule; example assumes compute:
        buildcommands {
            'dxc -nologo -E main -T cs_6_6 -O3 -Zpr -Fo "%{cfg.targetdir}/%{file.basename}.cso" "%{file.relpath}"'
        }
        buildoutputs { "%{cfg.targetdir}/%{file.basename}.cso" }
    filter {}


    targetdir ("%{wks.location}/Binaries/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")
    objdir ("%{wks.location}/Binaries/Intermediates/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")



----------------------------------------
-- All our Modules
----------------------------------------

include "Engine/Modules/Core/Core.lua"

group "Modules"
    include "Engine/Modules/Audio/Daft Punk.lua"
    include "Engine/Modules/ECS/Swarm.lua"
    include "Engine/Modules/Networking/Packet Ninja.lua"
    include "Engine/Modules/Platform/File Wizard.lua"
    include "Engine/Modules/Rendering/DaVinci.lua"
    include "Engine/Modules/Editor/Nitpick.lua"
    --project "EnkiTS Wrapper"
group ""


include "Engine/Runtime/Apocalypse.lua"
    
    
---------------------------------------
-- Auto-generate one project per example
---------------------------------------
group "Examples"
    CreateExampleProjects("Examples", function(name, dir)
        project(name)
        location "%{wks.location}/Examples"  
        kind "ConsoleApp"
        language "C++"

        files {
            dir .. "/**.h", 
            dir .. "/**.hpp", 
            dir .. "/**.c", 
            dir .. "/**.cpp",
            dir .. "/**.hlsl"
        }

        includedirs 
        {
            "%{wks.location}/Include",
            "%{wks.location}/Engine/Modules/Core/Include",
            "%{wks.location}/Engine/Modules/Platform/Include",
            "%{wks.location}/Engine/Modules/Rendering/Include",
        }

        postbuildcommands { "{COPYFILE} %[%{!wks.location}Lib/%{cfg.architecture}/%{cfg.buildcfg}/SDL3/SDL3.dll] %[%{!wks.location}/Binaries/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}]" }


        dependson { "ApocalypseRuntime", "imgui_sdl3_d3d", "SDL", "CoreModule" }
        links { "dxgi", "d3d12", "ApocalypseRuntime", "SDL", "CoreModule" }

    end)
group ""