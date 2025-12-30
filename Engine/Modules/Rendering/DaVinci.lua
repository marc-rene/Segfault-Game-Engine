group "Modules"
    project "DaVinci"
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

        dependson { "CORE_ENGINE_MODULE" }
        links 
        { 
            "CORE_ENGINE_MODULE",   "dxgi",     
            "d3d12",    "SDL",      "winmm",    
            "imm32",    "version",  "setupapi",
            "d3dcompiler",      "dxguid"
        } 
        
group ""


group "ThirdParty"
    -- SDL3 (no vulkan, only DirectX 12)
    externalproject "SDL"
        location "%{wks.location}/Engine/ThirdParty/SDL/VisualC/SDL"
        
        -- HEY THERE! IF YOU'RE HAVING SDL PROJECT ISSUES, CHECK SDL.vcxproj IN VISUALC FOLDER AND CHECK THAT <ProjectGuid> MATCHES
        uuid "81CE8DAF-EBB2-4761-8E45-B71ABCCA8C68" 
        kind "SharedLib"
        language "C"
        staticruntime "On"

        targetdir ("%{wks.location}/Lib/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")
        objdir ("%{wks.location}/Lib/Intermediates/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")

group ""