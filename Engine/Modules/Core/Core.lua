group "Modules"
    project "CoreModule"
        kind "StaticLib"
        language "C++"
        cppdialect "C++23"


        files {"%{prj.location}/**"}


        includedirs {"%{wks.location}/Engine/ThirdParty/Spdlog/include"}


        targetdir("%{wks.location}/Lib/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")
        objdir("%{wks.location}/Lib/Intermediates/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")
        
group ""
