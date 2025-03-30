workspace (WORKSPACE_NAME)
    startproject(START_PROJECT)
    -- 'arch'    can be overriden using --arch='x' flag
    -- 'system'  can be overriden using --os='x'   flag
    -- 'toolset' can be overriden using --cc='x'   flag (mostly...)
    filter { "system:windows", "action:vs2022" }
        toolset "msc-llvm-vs2022"
        if llvmdir and llvmversion then
            llvmdir     = os.getenv("LLVMInstallDir")
            llvmversion = os.getenv("LLVMToolsVersion")
        end
        flags { "MultiProcessorCompile" } -- NOTE: no equivalent in linux, must use makefiles and -j$NPROC
    filter {}
    filter { "system:windows", "action:gmake2" } -- This article should help in the future: https://stackoverflow.com/questions/29504627/adjusting-g-location-with-premake
        toolset "clang"
        if llvmdir and llvmversion then
            llvmdir     = os.getenv("LLVMInstallDir")
            llvmversion = os.getenv("LLVMToolsVersion")
            -- flags { "LinkTimeOptimization" } -- easy fix to switch from 'ar' to 'llvm-ar' 
            -- makesettings {
            --     "CC = "  .. '"' .. llvmdir .. "/bin/clang.exe"   .. '"' .. " --verbose",
            --     "CXX = " .. '"' .. llvmdir .. "/bin/clang++.exe" .. '"' .. " --verbose -ferror-limit=0 -fuse-ld=lld-link.exe",
            --     "LD = "  .. '"' .. llvmdir .. "/bin/ld.lld.exe"  .. '"' .. " --verbose",
            --     "AR = "  .. '"' .. llvmdir .. "/bin/llvm-ar.exe" .. '"' .. " v"
            -- }
            makesettings {
                "CC = "  .. '"' .. llvmdir .. "/bin/clang.exe"   .. '"' .. "",
                "CXX = " .. '"' .. llvmdir .. "/bin/clang++.exe" .. '"' .. " -ferror-limit=0 -fuse-ld=lld-link.exe",
                "LD = "  .. '"' .. llvmdir .. "/bin/ld.lld.exe"  .. '"' .. "",
                "AR = "  .. '"' .. llvmdir .. "/bin/llvm-ar.exe" .. '"' .. " v"
            }
        end
    filter {}
    -- #1 Link: https://askubuntu.com/questions/1508260/how-do-i-install-clang-18-on-ubuntu
    -- #2 Link: https://unix.stackexchange.com/questions/596226/how-to-change-clang-10-llvm-10-etc-to-clang-llvm-etc
    -- #3 I expect clang to be installed and symlinked on your machine, basically.
    -- #4 Why is everything easier on linux?!?
    filter "system:linux"
        toolset "clang"
        debugformat "Dwarf"
        -- if llvmdir then
        --     llvmdir = os.getenv("LLVMInstallDir")
        -- end
        -- makesettings {
        --     "CC = "  .. '"' .. llvmdir .. "/clang"   .. '"' .. " --verbose",
        --     "CXX = " .. '"' .. llvmdir .. "/clang++" .. '"' .. " --verbose -ferror-limit=0",
        --     "LD = "  .. '"' .. llvmdir .. "/ld.lld"  .. '"' .. " --verbose",
        --     "AR = "  .. '"' .. llvmdir .. "/llvm-ar-19" .. '"' .. " v"
        -- }
    filter {}


    configurations {
        "DebugLib",
        "ReleaseLib",
        "DebugDll",
        "ReleaseDll"
    }
    platforms {
        "x86",
        "amd64",
        "ARM",
        "ARM64"
    }


    -- We give priority to Platform over the 'arch' flag
    filter "platforms:x86"
        architecture "x86"
    filter "platforms:amd64"
        architecture "x86_64"
    filter "platforms:ARM"
        architecture "ARM"
    filter "platforms:ARM64"
        architecture "ARM64"
    filter {}

    filter "configurations:Debug*"
        defines { "DEBUG" }
        runtime  "Debug"
        symbols  "on"
        optimize "off"
        buildoptions { "-g" } 
    filter "configurations:Release*"
        defines { "NDEBUG" }
        runtime  "Release"
        symbols  "off"
        optimize "on"
    filter {}

    filter "architecture:x86"
        defines { "__x86__" }
    filter "architecture:x86_64"
        defines { "__x86_64__" }
    filter "system:linux"
        defines { "__linux__" }
    filter { "system:windows", "architecture:x86" }
        defines { "_WIN32" }
    filter { "system:windows", "architecture:x86_64" }
        defines { "_WIN64" }
    filter {}