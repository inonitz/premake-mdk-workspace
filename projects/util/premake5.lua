project "util"
    systemversion "latest"
    warnings      "extra"
    SpecifyGlobalProjectCXXVersion()
    -- Project Structure
    files { 
        "include/**.h",
        "source/**.c",
        "include/**.hpp",
        "source/**.cpp"
    }
    filter "files:stb_image.h" -- stb_image gives wayy to many warnings...
        warnings "Off"
    filter {}
    -- Specify Include Headers
    includedirs { 
        "include" 
    }

    -- Build Directories &// Structure
    SetupBuildDirectoriesForLibrary()

    -- Build Options
    filter "toolset:clang or toolset:gcc"
        buildoptions {
            "-msse3 -mavx"
        }
    filter "toolset:msc"
        buildoptions {
            "/arch:AVX"
        }
    filter {}

    -- Linking Options
    LinkToStandardLibraries()
    links {}
    
    -- Macros
    filter { "system:windows" }
        defines { "SYSTEM_WINDOWS" , "_CRT_SECURE_NO_WARNINGS" }
    filter {}
    filter { "configurations:*Lib" }
        defines { "UTIL_STATIC_DEFINE" }
    filter { "configurations:*Dll" }
        defines { "UTIL_EXPORTS" }
    filter {}


    -- Custom Pre &// Post build Actions