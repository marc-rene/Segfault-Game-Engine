group "Modules"
    project "NitPick"
        kind "StaticLib" 
        language "C++"
        cppdialect "C++23"

        files { "%{prj.location}/**" }

        includedirs 
        {
            "%{wks.location}/Engine/Modules/Core/Include",
            "%{wks.location}/Engine/Modules/Rendering/Include",
            "%{wks.location}/Engine/ThirdParty/ImGui",
        }
        
        targetdir ("%{wks.location}/Lib/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")
        objdir ("%{wks.location}/Lib/Intermediates/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")

        dependson { "CoreModule", "DaVinci", "imgui_sdl3_d3d", "SDL" }

        links { "CoreModule", "DaVinci" }
        
group ""



group "ThirdParty"
    project "imgui_sdl3_d3d"
        kind "StaticLib"
        language "C++"
        cppdialect "C++23"

        files {
            --"%{prj.location}/**",
            
            "%{wks.location}/Engine/ThirdParty/ImGui/*.*",       
            "%{wks.location}/Engine/ThirdParty/ImGui/backends/imgui_impl_dx11.*",       
            "%{wks.location}/Engine/ThirdParty/ImGui/backends/imgui_impl_dx12.*",       
            "%{wks.location}/Engine/ThirdParty/ImGui/backends/imgui_impl_sdl3.*",       
            
        }
        
        includedirs {
            "%{wks.location}/Engine/ThirdParty/ImGui",
            "%{wks.location}/Engine/ThirdParty/SDL/include",
        }
        
        targetdir ("%{wks.location}/Lib/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")
        objdir ("%{wks.location}/Lib/Intermediates/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")

group ""