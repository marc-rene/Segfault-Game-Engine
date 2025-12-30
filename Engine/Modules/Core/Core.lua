group "Modules"
    project "CORE_ENGINE_MODULE"
        kind "SharedLib" 
        language "C++"
        cppdialect "C++23"

        targetname "CORE_ENGINE_MODULE"

        files { "%{prj.location}/**" }

        includedirs 
        {
            "%{wks.location}/Engine/ThirdParty/Spdlog/include",
        }

        targetdir ("%{wks.location}/Lib/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")
        objdir ("%{wks.location}/Lib/Intermediates/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")
group ""