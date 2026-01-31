project "ApocalypseRuntime"
    kind "StaticLib"
    language "C++"
    cppdialect "C++23"
    
    
    files { "%{prj.location}/**" }
    
    includedirs {
        "%{wks.location}/Engine/Modules",
        "%{wks.location}/Engine/Modules/Core/Include",
        "%{wks.location}/Engine/Modules/Audio/Include",
        "%{wks.location}/Engine/Modules/ECS/Include",
        "%{wks.location}/Engine/Modules/Editor/Include",
        "%{wks.location}/Engine/Modules/Networking/Include",
        "%{wks.location}/Engine/Modules/Platform/Include",
        "%{wks.location}/Engine/Modules/Rendering/Include",
        "%{wks.location}/Engine/ThirdParty/DirectX_Headers/include/directx",
        --TODO: make the Core a static lib or dll to import instead of including all folders 
        "%{wks.location}/Engine/ThirdParty/Spdlog/include",
    }


    
    targetdir ("%{wks.location}/Lib/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")
    objdir ("%{wks.location}/Lib/Intermediates/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")
    
    links { 
        "FileWizard",   "PacketNinja",  "DaftPunk",
        "Swarm",        "DaVinci",      "NitPick",
    } 

    dependson { 
        "FileWizard",   "PacketNinja",  "DaftPunk",
        "Swarm",        "DaVinci",      "NitPick",
    } 