project "Segfault Editor"
kind "ConsoleApp"
language "C++"
cppdialect "C++23"
targetdir "Binaries/%{cfg.buildcfg}"
defines {}
files {"src/**.h", "src/**.cpp", "src/**.hpp", "Vendor/imgui/*.cpp", "Vendor/imgui/*.h", "Vendor/imgui/*.hpp",
       "Vendor/imgui/backends/imgui_impl_dx12.cpp", "Vendor/imgui/backends/imgui_impl_dx12.h",
       "Vendor/imgui/backends/imgui_impl_sdl3.cpp", "Vendor/imgui/backends/imgui_impl_sdl3.h"}

includedirs {"$(SolutionDir)SegFault Editor\\Vendor\\imgui",
             "$(SolutionDir)SegFault Editor\\Vendor\\SDL\\include",

             "$(SolutionDir)Misc\\File IO\\include",
             "$(SolutionDir)Misc\\Logging\\include",
             "$(SolutionDir)Misc\\Logging\\spdlog\\include", -- spdlog weirdness
             "$(SolutionDir)Misc\\Networking\\include", 
             "$(SolutionDir)Misc\\Audio\\include", 
             
             "$(SolutionDir)Misc\\Math\\DirectXMath\\Inc",
             "$(SolutionDir)Misc\\Math\\DirectXMathExtended\\include",}

libdirs {"$(SolutionDir)SegFault Editor\\Vendor\\SDL\\VisualC\\x64\\Debug"}

-- Link our modules
links {"Tout", "File_Wizard", "Packet_Ninja", "Daft_Punk", "d3d12", "SDL3"}

targetdir("../Binaries/" .. OutputDir .. "/%{prj.name}")
objdir("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

filter "system:windows"
systemversion "latest"
defines {}

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
