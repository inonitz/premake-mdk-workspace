workspace (WORKSPACE_NAME)
    startproject(START_PROJECT)
    -- default system parameters, required for proper folder structure.
    system          "windows"  -- can be overriden using --os='x'   flag
    architecture    "x86_64"   -- can be overriden using --arch='x' flag
    toolset "msc-llvm-vs2022"
    llvmversion "19"


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