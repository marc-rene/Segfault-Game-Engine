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
            "%{wks.location}/Engine/ThirdParty/DirectX_Headers/include/directx",
            "%{wks.location}/Engine/ThirdParty/SDL/include",
            "%{wks.location}/Engine/ThirdParty/NVRHI/include",
        }

        targetdir ("%{wks.location}/Lib/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")
        objdir ("%{wks.location}/Lib/Intermediates/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")

        dependson { "CoreModule", "SDL"}

        libdirs
        {
            "%{wks.location}/Lib/%{cfg.architecture}/%{cfg.buildcfg}/NVRHI/%{cfg.buildcfg}"
        }

        links 
        { 
            "dxgi",     "d3d12",    "winmm",    
            "imm32",    "version",  "setupapi", 
            "d3dcompiler",          "dxguid",
            "nvrhi",
        } 
        
group ""

group "ThirdParty"
    externalproject  "SDL"
        location "%{wks.location}/Engine/ThirdParty/SDL/VisualC/SDL"
        uuid "81CE8DAF-EBB2-4761-8E45-B71ABCCA8C68" 
        kind "StaticLib"
        language "C"
        staticruntime "On"

        targetdir ("%{wks.location}/Lib/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")
        objdir ("%{wks.location}/Lib/Intermediates/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")



    project "NVRHI"
        location "%{wks.location}/Engine/ThirdParty/NVRHI"
        kind "Utility"
        language "C++"
        

        prebuildcommands 
        {
            "cmake -B build -DNVRHI_BUILD_SHARED=1 -DNVRHI_WITH_DX11=1 -DNVRHI_WITH_DX12=1 -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=%[%{wks.location}\\Lib\\%{cfg.architecture}\\%{cfg.buildcfg}\\%{prj.name}] -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=%[%{wks.location}\\Lib\\%{cfg.architecture}\\%{cfg.buildcfg}\\%{prj.name}] -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=%[%{wks.location}\\Lib\\%{cfg.architecture}\\%{cfg.buildcfg}\\%{prj.name}]",
        }

        
        postbuildcommands  
        {
            "cmake --build build --config %{cfg.buildcfg}"
        }

        --targetdir ("%{wks.location}/Lib/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")
        --objdir ("%{wks.location}/Lib/Intermediates/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")