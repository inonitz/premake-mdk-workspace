-- Build Configuration
BUILD_BINARY_DIRECTORY_GENERIC = "/build/bin/%{cfg.buildcfg}_%{cfg.platform}_%{prj.name}"
BUILD_OBJECT_DIRECTORY_GENERIC = "/build/obj/%{cfg.buildcfg}_%{cfg.platform}_%{prj.name}"
BUILD_BINARY_DIRECTORY         = "/build/bin/%{cfg.buildcfg}_%{cfg.platform}"
COMPILE_COMMANDS_DIRECTORY     = _MAIN_SCRIPT_DIR .. "/.vscode/compile_commands"

AbsoluteProjectBinaryPath = function(projname)
    return _MAIN_SCRIPT_DIR .. BUILD_BINARY_DIRECTORY .. "_" .. projname
end

-- Thanks to: https://stackoverflow.com/questions/1426954/split-string-in-lua
function SplitStringOnSeparator(inputstr, sep)
    if sep == nil then
        sep = "%s"
    end
    local t = {}
    for str in string.gmatch(inputstr, "([^".. sep .."]+)") do
        table.insert(t, str)
    end
    return t
end




-- The Idea behind this is that the debug configuration on 'workspace.lua'
-- is not enough for lldb on windows. I need to manually tell the linker to generate pdb files
-- (makes sense all in all, gcc doesn't need pdb files to debug programs compiled with -g)
-- Needs Testing with gdb on linux / WSL
SetupLinkingFlagsForDebugInfo = function()
    local pdbpath = '"' .. _MAIN_SCRIPT_DIR .. BUILD_BINARY_DIRECTORY_GENERIC ..  "/%{prj.name}.pdb" .. '"'


    filter { "system:windows", "configurations:Debug* or configurations:RelWithDbgInfo*", "action:gmake or action:ninja" }
        linkoptions( "-g -Wl,--pdb=-Wl,--pdb=" .. pdbpath )
    filter {}
    -- Incomplete and untested, when working with vs2022 figure it out
    filter { "system:windows", "configurations:Debug* or configurations:RelWithDbgInfo*", "action:vs2022" }
        symbolspath( pdbpath )
    filter {}
end


SetupBuildDirectoriesForLibrary = function()
    filter "configurations:*Lib"
        kind "StaticLib"
        targetdir (_MAIN_SCRIPT_DIR .. BUILD_BINARY_DIRECTORY_GENERIC)
        objdir    (_MAIN_SCRIPT_DIR .. BUILD_OBJECT_DIRECTORY_GENERIC)
    filter {}

    filter "configurations:*Dll"
        kind "SharedLib"
        targetdir (_MAIN_SCRIPT_DIR .. BUILD_BINARY_DIRECTORY_GENERIC)
        objdir    (_MAIN_SCRIPT_DIR .. BUILD_OBJECT_DIRECTORY_GENERIC)
        linkoptions {
            "-Wl,-rpath=."
        }
        flags { "RelativeLinks" }
    filter {}


    filter { "configurations:*Dll", "system:windows" }
        -- Ninja builds in the root folder for each project, i.e 'root/'
        -- while make moves to each projects' directory and builds relative to 'root/projects/project_name'
        -- This in turn causes the --out-implib to be incorrect (specifically because cfg.linktarget.abspath is not actually the absolute path, only relative to 'root')
        -- For more info on the last comment (^^^) see: https://github.com/premake/premake-core/issues/94#event-1245523356
        -- Anyway, this option helps us bypass whatever the compiler emitted using premake5' cfg (The path below is Absolute).
        linkoptions {
            "-Wl,--out-implib=" .. _MAIN_SCRIPT_DIR .. BUILD_BINARY_DIRECTORY_GENERIC .. "/%{cfg.linktarget.name}"
        }
    filter {}
    SetupLinkingFlagsForDebugInfo()
end


SetupBuildDirectoriesForExecutable = function()
    filter "configurations:*Lib"
        targetdir (_MAIN_SCRIPT_DIR .. BUILD_BINARY_DIRECTORY_GENERIC)
        objdir    (_MAIN_SCRIPT_DIR .. BUILD_OBJECT_DIRECTORY_GENERIC)
    filter {}

    filter "configurations:*Dll"
        targetdir (_MAIN_SCRIPT_DIR .. BUILD_BINARY_DIRECTORY_GENERIC)
        objdir    (_MAIN_SCRIPT_DIR .. BUILD_OBJECT_DIRECTORY_GENERIC)
        linkoptions {
            "-Wl,-rpath=."
        }
        flags { "RelativeLinks" }
    filter {}
    SetupLinkingFlagsForDebugInfo()
end


IncludeProjectHeaders = function(ProjectName)
    includedirs { _MAIN_SCRIPT_DIR .. "/projects/" .. ProjectName .. "/include" }
end


LinkProjectLibrary = function(ProjectName)
    links { ProjectName }
end


LinkUtil2Library = function()
    LinkProjectLibrary("util2")
    filter { "configurations:*Lib" }
        defines { "UTIL2_STATIC_DEFINE" }
    filter {}
end

LinkGLFWLibrary = function()
    LinkProjectLibrary("glfw34")
    filter { "system:windows", "configurations:*Lib" }
        links { 
            "user32",
            "imm32",
            "gdi32",
            "shell32"
        }
    filter { "system:linux", "configurations:*Lib" }
        links { 
            "pthread", 
            "dl", 
            "X11", 
            "Xcursor", 
            "Xxf86vm", 
            "Xrandr" 
        }
    filter {}
end

LinkGLBindingLibraries = function()
    LinkProjectLibrary("glbinding")
    filter { "configurations:*Lib" }
        defines { "GLBINDING_STATIC_DEFINE" }
    filter {}
    LinkProjectLibrary("glbinding-aux")
    filter { "configurations:*Lib" }
        defines { "GLBINDING_AUX_STATIC_DEFINE" }
    filter {}
end

LinkImGuiLibrary = function()
    LinkProjectLibrary("imgui")
    filter { "configurations:*Lib" }
        defines { "IMGUI_STATIC_DEFINE" }

    filter { "configurations:*Lib", "system:windows" }
        links { "imm32" }

    filter {}
end

LinkAWC2Library = function()
    LinkProjectLibrary("awc2")
    filter { "configurations:*Lib" }
        defines { "AWC2_STATIC_DEFINE" }
    filter {}
end



PreBuildCopyBuildTargetCompileCommandsToFolder = function()
    prebuildcommands { 
        "cp -rf " .. COMPILE_COMMANDS_DIRECTORY .. "/%{cfg.shortname}.json " .. COMPILE_COMMANDS_DIRECTORY .. "/compile_commands.json"
        -- "{copyfile} %[../../.vscode/compile_commands/%{cfg.shortname}.json] %[../../.vscode/compile_commands/compile_commands.json]"
    }
end


PostBuildCommmandsForExecutable = function()
    -- Post build commands
    filter { "action:gmake or action:ninja", "configurations:*Dll" }
        local collectiveBinaryDir = _MAIN_SCRIPT_DIR .. BUILD_BINARY_DIRECTORY
        if not os.isdir(collectiveBinaryDir) then
            postbuildcommands{ "mkdir -p " .. collectiveBinaryDir }
        end
        for _, projpath in ipairs(PROJECT_LIST) do
            local projname = SplitStringOnSeparator(projpath, "/")
            projname = projname[#projname] -- Last String in the projname list 
            postbuildcommands{ "cp -rf " .. AbsoluteProjectBinaryPath(projname) .. "/* " .. collectiveBinaryDir }
        end
            -- -- postbuildcommands{ "cp -rf %[../../%{BUILD_BINARY_DIRECTORY}_googletest/*   ] %[../../%{BUILD_BINARY_DIRECTORY}]" }
            -- -- postbuildcommands{ "cp -rf %[../../%{BUILD_BINARY_DIRECTORY}_benchmark194/* ] %[../../%{BUILD_BINARY_DIRECTORY}]" }
            -- -- postbuildcommands{ "cp -rf %[%{cfg.buildtarget.directory}] %[../../%{BUILD_BINARY_DIRECTORY}]" }
            -- postbuildcommands{ "cp -rf ../.." .. "%{BUILD_BINARY_DIRECTORY}_googletest/*   " .. "../.." .. "%{BUILD_BINARY_DIRECTORY}" }
            -- postbuildcommands{ "cp -rf ../.." .. "%{BUILD_BINARY_DIRECTORY}_benchmark194/* " .. "../.." .. "%{BUILD_BINARY_DIRECTORY}" }
            -- postbuildcommands{ "cp -rf %{cfg.buildtarget.directory}/* "                      .. "../.." .. "%{BUILD_BINARY_DIRECTORY}" }
    filter {}
end