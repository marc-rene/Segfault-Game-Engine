local M = {}

function UseCommonIncludeDirs()
    includedirs {
        "%{wks.location}/Src",
        "%{wks.location}/Src/Core/Math",
        "%{wks.location}/Src/Core/Spdlog/include",
        "%{wks.location}/Src/ECS",
        "%{wks.location}/Src/Gameplay",
        "%{wks.location}/Src/Gameplay/Navigation/Recast/Recast/Include",
        "%{wks.location}/Src/Gameplay/Navigation/Recast/Detour/Include",
        "%{wks.location}/Src/Gameplay/Navigation/Recast/DetourTileCache/Include",
        "%{wks.location}/Src/Networking",
        "%{wks.location}/Src/Networking/Game Networking Sockets/include",
        "%{wks.location}/Src/Platform",
        "%{wks.location}/Src/Platform/EnkiTS/src",
        "%{wks.location}/Src/Platform/SDL/include",
        "%{wks.location}/Src/Rendering",
        "%{wks.location}/Src/Rendering/Imgui",
        "%{wks.location}/Src/Rendering/Imgui/backends",
        "%{wks.location}/Src/Rendering/Renderer",
        "%{wks.location}/Src/Resources",
        "%{wks.location}/Src/Runtime",

        "%{wks.location}/ThirdParty/DirectX 12 Memory Allocator/include",
        "%{wks.location}/ThirdParty/DirectX 12 Toolkit/Inc",
        "%{wks.location}/ThirdParty/EnkiTS/src",
        "%{wks.location}/ThirdParty/Game Networking Sockets/include",
        "%{wks.location}/ThirdParty/ImGui",
        "%{wks.location}/ThirdParty/Recast Navigation/Detour/Include",
        "%{wks.location}/ThirdParty/Recast Navigation/Recast/Include",
        "%{wks.location}/ThirdParty/Recast Navigation/DebugUtils/Include",
        "%{wks.location}/ThirdParty/SDL/include",
        "%{wks.location}/ThirdParty/Spdlog/include",
    }

    filter "system:windows"
        links { "dxgi", "d3d12" }
    filter {}

end

SDL3_Path = "%{wks.location}/Binaries/%{cfg.architecture}/%{cfg.buildcfg}/SDL/SDL3"
Engine_Path = "%{wks.location}/Binaries/%{cfg.architecture}/%{cfg.buildcfg}/Segfault Game Engine/SegfaultGameEngine"


function UseApocalypseEngine()
    -- Link against the static lib
    links { Engine_Path }
    dependson { "Segfault Game Engine", "imgui_sdl3_dx12", "SDL" }
end



function SetOutputDirs()
    targetdir ("%{wks.location}/Binaries/" .. OutputDir .. "/%{prj.name}")
    objdir ("%{wks.location}/Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")
end



-- Discover subfolders under examplesRoot and create a project per folder.
-- callback(name, dir) is called for each discovered example.
function CreateExampleProjects(examplesRoot, callback)
    local dirs = os.matchdirs(examplesRoot .. "/*")
    table.sort(dirs) -- stable order in IDE
    for _, dir in ipairs(dirs) do
        local name = path.getname(dir)
        callback(name, dir)
    end
end

return M