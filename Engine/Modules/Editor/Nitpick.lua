group "Modules"
    project "Nitpick"
        kind "SharedLib" 
        language "C++"
        cppdialect "C++23"

        files { "%{prj.location}/**" }

        includedirs 
        {
            "%{wks.location}/Engine/Modules/Core/Include",
            "%{wks.location}/Engine/ThirdParty/DirectX_12_Memory_Allocator/include",
            "%{wks.location}/Engine/ThirdParty/DirectX_12_Toolkit/Inc",
            "%{wks.location}/Engine/ThirdParty/DirectX_Math/Inc",
            "%{wks.location}/Engine/ThirdParty/SDL/include",
        }
        
        targetdir ("%{wks.location}/Lib/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")
        objdir ("%{wks.location}/Lib/Intermediates/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")

        dependson { "DaVinci", "imgui_sdl3_d3d" }
        links { "DaVinci", "imgui_sdl3_d3d" } 
        
group ""



group "ThirdParty"
    project "imgui_sdl3_d3d"
        kind "StaticLib"
        language "C++"
        cppdialect "C++20"

        files {
            "%{prj.location}/**",
            
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

        dependson { "SDL" }
group ""