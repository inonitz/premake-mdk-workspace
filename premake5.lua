-- Thankss to: https://stackoverflow.com/questions/75490124/how-to-require-a-file-with-a-dot-in-the-name
local function __custom_searcher(module_name)
    -- Use "/" instead of "." as directory separator
    local path, err = package.searchpath(module_name, package.path, "/")
    if path then
        return assert(loadfile(path))
    end
    return err
end


table.insert(package.searchers, __custom_searcher)
local function __custom_require(module_name)
    return assert(loadfile(assert(package.searchpath(module_name, package.path, "/"))))
end


-- __custom_require(".vscode/export-compile-comms")
-- __custom_require(".vscode/ecc")
-- require(".vscode/ecc")
require(".vscode/export-compile-comms")


-- Makes a path relative to the folder containing this script file.
ROOT_PATH = function(path)
    return string.format("%s/%s", _MAIN_SCRIPT_DIR, path)
end


PROJECT_DIR     = ROOT_PATH("projects")
DEPENDENCY_DIR  = ROOT_PATH("dependencies")
WORKSPACE_NAME  = "example-awc2-program"
START_PROJECT   = "program"


BUILD_BINARY_DIRECTORY_GENERIC = "/build/bin/%{cfg.buildcfg}_%{cfg.platform}_%{prj.name}"
BUILD_OBJECT_DIRECTORY_GENERIC = "/build/obj/%{cfg.buildcfg}_%{cfg.platform}_%{prj.name}"
BUILD_BINARY_DIRECTORY = "/build/bin/%{cfg.buildcfg}_%{cfg.platform}"
BUILD_BINARY_DIRECTORY_ABSOLUTE = _MAIN_SCRIPT_DIR .. BUILD_BINARY_DIRECTORY


SpecifyGlobalProjectCXXVersion = function()
--     filter "toolset:not gcc"
--         filter "files:**.c"
--             buildoptions { "-std=c11" }
--         filter "files:**.cpp"
--             buildoptions { "-std=c++17" }
--     filter ""
--     filter "toolset:gcc"
--         filter "files:**.c"
--             buildoptions { "-std=gnu11" }
--         filter "files:**.cpp"
--             buildoptions { "-std=gnu++17" }
--     filter ""
--     filter {}
    filter { "toolset:not gcc" }
        cdialect "C11"
        cppdialect "C++17"
    filter { "toolset:gcc" }
        cdialect "gnu11"
        cppdialect "gnu++17"
    filter {}


end


SetupBuildDirectoriesForLibrary = function()
    filter "configurations:*Lib"
        kind "StaticLib"
        targetdir (_MAIN_SCRIPT_DIR .. BUILD_BINARY_DIRECTORY_GENERIC)
        objdir    (_MAIN_SCRIPT_DIR .. BUILD_OBJECT_DIRECTORY_GENERIC)
    filter ""

    filter "configurations:*Dll"
        kind "SharedLib"
        targetdir (_MAIN_SCRIPT_DIR .. BUILD_BINARY_DIRECTORY_GENERIC)
        objdir    (_MAIN_SCRIPT_DIR .. BUILD_OBJECT_DIRECTORY_GENERIC)
    filter ""
    filter {}
end


SetupBuildDirectoriesForExecutable = function()
    filter "configurations:*Lib"
        targetdir (_MAIN_SCRIPT_DIR .. BUILD_BINARY_DIRECTORY_GENERIC)
        objdir    (_MAIN_SCRIPT_DIR .. BUILD_OBJECT_DIRECTORY_GENERIC)
    filter ""

    filter "configurations:*Dll"
        targetdir (_MAIN_SCRIPT_DIR .. BUILD_BINARY_DIRECTORY_GENERIC)
        objdir    (_MAIN_SCRIPT_DIR .. BUILD_OBJECT_DIRECTORY_GENERIC)
    filter ""
    filter {}
end



-- The reason for this mess: https://groups.google.com/g/llvm-dev/c/WA1vKn9zDtM
LinkToStandardLibraries = function()
    -- Directly Taken from: https://learn.microsoft.com/en-us/cpp/c-runtime-library/crt-library-features?view=msvc-170
    -- Static Debug:
    -- Libraries: libucrtd.lib libvcruntimed.lib libcmtd.lib libcpmtd.lib
    -- Macros: _DEBUG, _MT
    filter { "system:windows", "action:gmake2", "configurations:DebugLib" }
        buildoptions { "-fms-runtime-lib=static_dbg" }
        defines { "_MT", "_DEBUG" }
        linkoptions { "-Wl,/nodefaultlib,kernel32.lib,libcmtd.lib,libcpmtd.lib,libvcruntimed.lib,libucrtd.lib" }

    -- Static Release: 
    -- Libraries: libucrt.lib libvcruntime.lib libcmt.lib libcpmt.lib
    -- Macros: _MT
    filter { "system:windows", "action:gmake2", "configurations:ReleaseLib" }
        buildoptions { "-fms-runtime-lib=static" }
        defines { "_MT" }
        linkoptions { "-Wl,/nodefaultlib,kernel32.lib,libcmt.lib,libcpmt.lib,libvcruntime.lib,libucrt.lib" }


    -- Dll Debug: 
    -- Libraries: ucrtd.lib vcruntimed.lib msvcrtd.lib msvcprtd.lib 
    -- Macros: _DEBUG, _MT, _DLL
    filter { "system:windows", "action:gmake2", "configurations:DebugDll" }
        buildoptions { "-fms-runtime-lib=dll_dbg" }
        defines { "_MT", "_DEBUG", "_DLL" }
        linkoptions { "-Wl,/nodefaultlib,kernel32.lib,msvcrtd.lib,msvcprtd.lib,vcruntimed.lib,ucrtd.lib" }

    -- Dll Release: 
    -- Libraries: ucrt.lib vcruntime.lib msvcrt.lib msvcprt.lib
    -- Macros: _MT, _DLL
    filter { "system:windows", "action:gmake2", "configurations:ReleaseDll" }
        buildoptions { "-fms-runtime-lib=dll" }
        defines { "_MT", "_DLL" }
        linkoptions { "-Wl,/nodefaultlib,kernel32.lib,msvcrt.lib,msvcprt.lib,vcruntime.lib,ucrt.lib" }

    filter {}
end




-- try to use these generic functions to include & link every project, right now the only ones popping up on program is glfw (also for awc2 ...)
IncludeProjectHeaders = function(ProjectName)
    includedirs { ROOT_PATH("projects/" .. ProjectName .. "/include") }
end

LinkProjectLibrary = function(ProjectName)
    links { ProjectName }
    filter {}
end


LinkUtilLibrary = function()
    LinkProjectLibrary("util")
    filter { "configurations:*Lib" }
        defines { "UTIL_STATIC_DEFINE" }
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
    filter { "system:windows", "configurations:*Lib" }
        defines { "IMGUI_STATIC_DEFINE" }
        links { "imm32" }
    filter { "system:not windows", "configurations:*Lib" }
        defines { "IMGUI_STATIC_DEFINE" }
    filter {}
end

LinkAWC2Library = function()
    LinkProjectLibrary("awc2")
    filter { "configurations:*Lib" }
        defines { "AWC2_STATIC_DEFINE" }
    filter {}
end




-- VERY IMPORTANT! IT MIGHT NOT LOOK LIKE MUCH
-- BUT ITS DOING LITERALLY EVERYTHING IN THIS WORKSPACE
include "workspace.lua"
include "projects.lua"
for _, path in ipairs(PROJECTS) do
    include(path .. "/premake5.lua")
end


-- Add Option to compile for a certain architecture, may not necessarily match the machine architecture
-- [NOTE]: Deprecated and not used anymore
-- newoption {
--     trigger     = "arch",
--     description = "Manually Specify the architecture of the project",
--     value       = "x86_64",
--     allowed = {
--         { "universal", "Universal Binaries Supported by ios & macOS " },
--         { "i386",                           "Alias for x86" },
--         { "x86",                                         "" },
--         { "amd64",                       "Alias for x86_64" },
--         { "x86_64",                                      "" },
--         { "ARM",                                         "" },
--         { "ARM64",                                       "" },
--         { "armv5",   "Only supported in VSAndroid projects" },
--         { "armv7",   "Only supported in VSAndroid projects" },
--         { "aarch64", "Only supported in VSAndroid projects" },
--         { "mips",    "Only supported in VSAndroid projects" },
--         { "mips64",  "Only supported in VSAndroid projects" },
--     },
--     default = "x86_64"
-- }

getProjectNames = function()
    folders = {}
    for _, projpath in ipairs(PROJECTS) do
        local i, j = string.find(projpath, '/')
        local projname = string.sub(projpath, i + 1) 
        table.insert(folders, { projname, projpath })
    end
    return folders
end

newoption {
    trigger     = "proj",
    description = "Set this when using cleanproj",
    allowed = getProjectNames()
}



-- Rebuild Project Solutions' Function --
newaction {
    trigger     = "cleancfgs",
    description = "Remove all Project Solutions, Makefiles, Ninja build files, etc...",
    execute     = function ()
        local build_extensions = { "/Makefile", "/**.sln", "/**.vcxproj", "/**.vcxproj.filters", "/**.vcxproj.user", "/**.ninja", "/.ninja_deps", "/.ninja_log", "/.ninja_lock" }
        local ok, err
        table.insert(PROJECTS, ".")
        for _, path in ipairs(PROJECTS) do
            local flag = true
            printf("%s", path)

            for __, ext in ipairs(build_extensions) do
                ok, err = os.remove{ path .. ext }
                if ok then
                    printf("%-30s: %s", "Removed Project Config Files", path .. ext)
                else
                    printf("%-30s: %s", "Got Error", err)
                end
            end
            printf("----------------------------------------")
        end
        table.remove(PROJECTS)
        print("Done")
    end
}


-- Wipe Function --
newaction {
    trigger     = "cleanbuild",
    description = "Delete All Object Files & Executables created during the build process",
    execute     = function ()
        local dirs_to_delete = { "./build", "./.vs" }
        local ok, err
        printf("----------------------------------------")
        for _, path in ipairs(dirs_to_delete) do
            ok, err = os.rmdir(path)
            if ok then
                printf("%-30s: %s", "Removed Directory", path)
            else
                printf("%-30s: %s", "Got Error", err)
            end
        end
        printf("----------------------------------------")
        print("Done")
    end
}


-- Delete whatever was generated using export-compile-commands
newaction {
    trigger     = "cleanclangd",
    description = "Delete the compile_commands/* Directory",
    execute     = function ()
        local ok, err, path
        printf("----------------------------------------")
        path = "./.vscode/compile_commands"
        ok, err = os.rmdir(path)
        if ok then
            printf("%-30s: %s", "Removed Directory", path)
        else
            printf("%-30s: %s", "Got Error", err)
        end
        printf("----------------------------------------")
        print("Done")
    end
}


-- Clean Specific System--Architecture Build Function [TODO] --
newaction {
    trigger     = "CleanArch[TODO]",
    description = "Delete All Debug & Release files generated for a specific system-architecture (e.g windows-x86_64)",
    execute     = function ()
        os.remove("build/bin/Debug-") 
        os.rmdir("build/**")
        print("Done")
    end
}


newaction {
    trigger     = "cleanproj",
    description = "Delete All Files Generated for a specific project",
    allowed = getProjectNames(),
    execute = function()
        if _OPTIONS["proj"] then
            local specific_bin_dirs = os.matchdirs( "build/bin/*" .. _OPTIONS["proj"] .. "*" )
            local specific_obj_dirs = os.matchdirs( "build/obj/*" .. _OPTIONS["proj"] .. "*" )
            local status, maybeError
            for key, value in pairs(specific_bin_dirs) do
                printf("----------------------------------------")
                status, maybeError = os.rmdir(value)
                if status then
                    printf("%-30s: %s", "Removed Directory", value)
                else
                    printf("%-30s: %s", "Got Error", maybeError)
                end
                printf("----------------------------------------")
            end
            for key, value in pairs(specific_obj_dirs) do
                printf("----------------------------------------")
                status, maybeError = os.rmdir(value)
                if status then
                    printf("%-30s: %s", "Removed Directory", value)
                else
                    printf("%-30s: %s", "Got Error", maybeError)
                end
                printf("----------------------------------------")
            end
        else
            printf("--proj option wasn't specified - Can't Continue!")
        end
    end
}



newaction {
    trigger     = "cleanall",
    description = "Trigger the following actions: cleanprojectconfigs, cleanclangd, cleanbuild",
    execute = function()
        print("[ACTION] = [cleanll] Begin\n")
        os.execute("premake5 cleancfgs")
        os.execute("premake5 cleanclangd")
        os.execute("premake5 cleanbuild")
        print("[ACTION] = [cleanll] End\n")
    end
}


newaction {
    trigger     = "buildalldbg",
    description = "Trigger the following actions: export-compile-commands, gmake2, make config=debuglib_amd64",
    execute = function()
        print("[ACTION] = [buildall] Begin\n")
        os.execute("premake5 export-compile-commands")
        os.execute("premake5 gmake2")
        os.execute("make config=debuglib_amd64 -j 16")
        print("[ACTION] = [buildall] End\n")
    end
}


newaction {
    trigger     = "buildallrel",
    description = "Trigger the following actions: export-compile-commands, gmake2, make config=releaselib_amd64",
    execute = function()
        print("[ACTION] = [buildall] Begin\n")
        os.execute("premake5 export-compile-commands")
        os.execute("premake5 gmake2")
        os.execute("make config=releaselib_amd64 -j 16")
        print("[ACTION] = [buildall] End\n")
    end
}