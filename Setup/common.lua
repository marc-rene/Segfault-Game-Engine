local common = {}



function Use_Core_Include_Dirs()
    includedirs {
        "%{wks.location}/Src/Core/TypeDefinitions.hpp",
        "%{wks.location}/Src/Core/Log.hpp",
        "%{wks.location}/Src/Core/BaseModule.hpp",
    }
end



function SetOutputDirs()
    targetdir ("%{wks.location}/Binaries/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")
    objdir ("%{wks.location}/Binaries/Intermediates/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}")
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



return common