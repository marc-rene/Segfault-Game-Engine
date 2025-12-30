group "Modules"
    project "File Wizard"
        kind "SharedLib" 
        language "C++"
        cppdialect "C++23"

        targetname "FileWizard"

        files { "%{prj.location}/**" }

        includedirs 
        {
            "%{wks.location}/Engine/Modules/Core/Include",
        }
        
        targetdir ("%{wks.location}/Lib/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")
        objdir ("%{wks.location}/Lib/Intermediates/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")

        dependson { "CORE_ENGINE_MODULE" }
        links { "CORE_ENGINE_MODULE" } 
group ""