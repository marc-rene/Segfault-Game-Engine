project "File_Wizard"
kind "StaticLib"
language "C++"
cppdialect "C++23"
targetdir "Binaries/%{cfg.buildcfg}"
staticruntime "off"

files {"include/**.h", "src/**.cpp", "include/**.hpp", "src/**.c"}

includedirs {"$(SolutionDir)Misc\\File IO\\mINI\\src"}

