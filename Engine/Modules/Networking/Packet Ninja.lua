group "Modules"
    project "Packet Ninja"
        kind "SharedLib" 
        language "C++"
        cppdialect "C++23"

        targetname "PacketNinja"

        files { "%{prj.location}/**" }

        includedirs 
        {
            "%{wks.location}/Engine/Modules/Core/Include",
            "%{wks.location}/Engine/ThirdParty/Game_Networking_Sockets/include",
            --"%{wks.location}/Engine/ThirdParty/FMOD",
            --"%{wks.location}/Engine/ThirdParty/XAudio2",
        }
        
        targetdir ("%{wks.location}/Lib/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")
        objdir ("%{wks.location}/Lib/Intermediates/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")

        dependson { "CORE_ENGINE_MODULE" }
        links { "CORE_ENGINE_MODULE" } 
group ""