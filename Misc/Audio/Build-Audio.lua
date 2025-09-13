project "Daft_Punk"
kind "StaticLib"
language "C++"
cppdialect "C++23"
targetdir "Binaries/%{cfg.buildcfg}"
staticruntime "off"
links {"Tout"}

files {"include/**.h", "src/**.cpp", "include/**.hpp", "src/**.c"}

includedirs {
    "$(SolutionDir)Misc\\Logging\\include",
    "$(SolutionDir)Misc\\Logging\\spdlog\\include"}

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
