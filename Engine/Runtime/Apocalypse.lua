project "Apocalypse Runtime"
    kind "SharedLib"
    language "C++"
    cppdialect "C++23"
    
    targetname "ApocalypseRuntime"
    
    
    files { "%{prj.location}/**" }
    
    includedirs 
    {
        "%{wks.location}/Engine/Modules",
        "%{wks.location}/Engine/Modules/Core/Include",
        "%{wks.location}/Engine/Modules/Audio/Include",
        "%{wks.location}/Engine/Modules/ECS/Include",
        "%{wks.location}/Engine/Modules/Editor/Include",
        "%{wks.location}/Engine/Modules/Networking/Include",
        "%{wks.location}/Engine/Modules/Platform/Include",
        "%{wks.location}/Engine/Modules/Rendering/Include",
    }

    dependson 
    {
        "File Wizard",
        "Packet Ninja",
        "Daft Punk",
        "Swarm",
        "DaVinci",
        "NitPick"
    }
    
    targetdir ("%{wks.location}/Lib/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")
    objdir ("%{wks.location}/Lib/Intermediates/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")
    
    links 
    { 
        "FileWizard",   "PacketNinja",  "DaftPunk",
        "Swarm",        "DaVinci",      "NitPick",
    } 