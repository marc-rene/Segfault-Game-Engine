project "Daft_Punk"
    kind "StaticLib"
    language "C++"
    cppdialect "C++23"
    targetdir "Binaries/%{cfg.buildcfg}"
    staticruntime "off"
    
    files {"include/**.h", "src/**.cpp", "include/**.hpp", "src/**.c"}

    includedirs {
        "$(SolutionDir)Misc\\Logging\\include",
        "$(SolutionDir)Misc\\Logging\\spdlog\\include",
        "C:\\Program Files (x86)\\FMOD SoundSystem\\FMOD Studio API Windows\\api\\core\\inc", -- TODO: Come up with some way of finding this out automatically
        "C:\\Program Files (x86)\\FMOD SoundSystem\\FMOD Studio API Windows\\api\\fsbank\\inc", -- TODO: Come up with some way of finding this out automatically
        "C:\\Program Files (x86)\\FMOD SoundSystem\\FMOD Studio API Windows\\api\\studio\\inc", -- TODO: Come up with some way of finding this out automatically
    }

    links {
        "Tout",
        "C:\\Program Files (x86)\\FMOD SoundSystem\\FMOD Studio API Windows\\api\\core\\lib\\x64\\fmod_vc.lib",
        "C:\\Program Files (x86)\\FMOD SoundSystem\\FMOD Studio API Windows\\api\\fsbank\\lib\\x64\\fsbank_vc.lib",
        "C:\\Program Files (x86)\\FMOD SoundSystem\\FMOD Studio API Windows\\api\\studio\\lib\\x64\\fmodstudio_vc.lib"
    }
    
filter "configurations:Debug"
defines {"DEBUG"}
runtime "Debug"
symbols "On"
staticruntime "off"

filter "configurations:Release"
defines {"RELEASE"}
runtime "Release"
optimize "On"
symbols "On"
staticruntime "on"

filter "configurations:Dist"
defines {"DIST"}
runtime "Release"
optimize "On"
symbols "Off"
staticruntime "on"

