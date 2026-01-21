group "Modules"
    project "DaVinci"
        kind "StaticLib"
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

        dependson { "CoreModule" }

        links 
        { 
            "dxgi",     "d3d12",    "winmm",    
            "imm32",    "version",  "setupapi", 
            "d3dcompiler",          "dxguid"
        } 
        
group ""


