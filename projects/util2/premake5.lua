project "util2"
    systemversion "latest"
    warnings      "extra"
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
    buildoptions { "-pthread" }

    -- Linking Options
    LinkToStandardLibraries()
    links { "pthread" }
    
    
    -- Macros
    -- filter { "system:windows" }
    --     defines { "SYSTEM_WINDOWS" , "_CRT_SECURE_NO_WARNINGS" }
    -- filter {}
    filter { "configurations:*Lib" }
        defines { "UTIL2_STATIC_DEFINE" }
    filter { "configurations:*Dll" }
        defines { "UTIL2_EXPORTS" }
    filter {}


    -- Custom Pre &// Post build Actions