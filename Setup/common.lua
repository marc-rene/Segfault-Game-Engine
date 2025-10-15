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
        "%{wks.location}/Rendering/Renderer",
        "%{wks.location}/Resources",
        "%{wks.location}/Runtime",
    }

    filter "system:windows"
        links { "dxgi", "d3d12" }
    filter {}
end

function UseApocalypseEngine()
    -- Link against the static lib
    links { "Segfault Game Engine" }

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