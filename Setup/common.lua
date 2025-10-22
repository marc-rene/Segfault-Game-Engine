local M = {}

function UseCommonIncludeDirs()
    includedirs {
        "%{wks.location}/Include",
        
        "%{wks.location}/ThirdParty/DirectX 12 Memory Allocator/include",
        "%{wks.location}/ThirdParty/DirectX 12 Toolkit/Inc",
        "%{wks.location}/ThirdParty/DirectX Math/Inc",
        "%{wks.location}/ThirdParty/EnkiTS/src",
        "%{wks.location}/ThirdParty/Game Networking Sockets/include",
        "%{wks.location}/ThirdParty/ImGui",
        "%{wks.location}/ThirdParty/ImGui/backends",
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