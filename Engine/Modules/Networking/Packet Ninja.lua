group "Modules"
    project "PacketNinja"
        kind "StaticLib" 
        language "C++"
        cppdialect "C++23"

        files { "%{prj.location}/**" }

        includedirs 
        {
            "%{wks.location}/Engine/Modules/Core/Include",
            "%{wks.location}/Engine/ThirdParty/Game_Networking_Sockets/include",
        }
        
        targetdir ("%{wks.location}/Lib/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")
        objdir ("%{wks.location}/Lib/Intermediates/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")

        dependson { "CoreModule" }
        
group ""