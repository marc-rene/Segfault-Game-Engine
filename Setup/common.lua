local M = {}

function UseCommonIncludeDirs()
    includedirs {
        "%{wks.location}/Audio",
        "%{wks.location}/Core",
        "%{wks.location}/Core/Math",
        "%{wks.location}/Core/Spdlog/include",
        "%{wks.location}/ECS",
        "%{wks.location}/Gameplay",
        "%{wks.location}/Gameplay/Navigation/Recast/Recast/Include",
        "%{wks.location}/Gameplay/Navigation/Recast/Detour/Include",
        "%{wks.location}/Gameplay/Navigation/Recast/DetourTileCache/Include",
        "%{wks.location}/Networking",
        "%{wks.location}/Networking/Game Networking Sockets/include",
        "%{wks.location}/Platform",
        "%{wks.location}/Platform/EnkiTS/src",
        "%{wks.location}/Platform/SDL/include",
        "%{wks.location}/Rendering",
        "%{wks.location}/Rendering/DirectX 12/Memory Allocator/include",
        "%{wks.location}/Rendering/Imgui",
        "%{wks.location}/Rendering/Imgui/backends",
        "%{wks.location}/Rendering/Renderer",
        "%{wks.location}/Resources",
        "%{wks.location}/Runtime",
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