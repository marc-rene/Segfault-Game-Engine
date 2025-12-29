local M = {}

function UseCommonIncludeDirs()
    includedirs {
        "%{wks.location}/Include",
        "%{wks.location}/ThirdParty/DirectX_12_Memory_Allocator/include",
        "%{wks.location}/ThirdParty/DirectX_12_Toolkit/Inc",
        "%{wks.location}/ThirdParty/DirectX_Math/Inc",
        "%{wks.location}/ThirdParty/EnkiTS/src",
        "%{wks.location}/ThirdParty/Game_Networking_Sockets/include",
        "%{wks.location}/ThirdParty/ImGui",
        "%{wks.location}/ThirdParty/ImGui/backends",
        "%{wks.location}/ThirdParty/Recast_Navigation/Detour/Include",
        "%{wks.location}/ThirdParty/Recast_Navigation/Recast/Include",
        "%{wks.location}/ThirdParty/Recast_Navigation/DebugUtils/Include",
        "%{wks.location}/ThirdParty/SDL/include",
        "%{wks.location}/ThirdParty/Spdlog/include",
    }

    filter "system:windows"
        links { "dxgi", "d3d12", "d3d11" }
    filter {}

end


Engine_Path = "%{wks.location}/Binaries/%{cfg.architecture}/%{cfg.buildcfg}/Segfault Game Engine/SegfaultGameEngine"


function UseApocalypseEngine()
    -- Link against the static lib
    links { Engine_Path }
    dependson { "Segfault Game Engine", "imgui_sdl3_d3d", "SDL" }
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