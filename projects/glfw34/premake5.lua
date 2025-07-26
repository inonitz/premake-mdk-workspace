project "glfw34"
    systemversion "latest"
    warnings      "extra"
    rtti          "On"
    -- Project Structure
    files {
        "source/**.h",
        "source/**.c"
    }

    -- Specify Include Headers
    -- Other Project Includes Defined here...
    -- e.g IncludeProjectHeaders(...)
    includedirs { "include" }

    -- Build Directories &// Structure
    SetupBuildDirectoriesForLibrary()

    -- Build Options
    buildoptions {}


    -- Linking Options
    LinkToStandardLibraries()
    filter { "system:windows" }
        links { "user32", "imm32", "gdi32", "shell32" }
    
    filter { "system:linux" }
        links { "pthread", "dl", "X11", "Xcursor", "Xxf86vm", "Xrandr" }
    filter {}


    -- Macros
    filter { "system:linux" }
        defines { 
            "_GLFW_X11",
            "_XOPEN_SOURCE=700"
        }
    filter {}
    filter { "system:windows" }
        defines { 
            "_GLFW_WIN32"
            -- "_CRT_SECURE_NO_WARNINGS" 
        }
    filter {}
    filter { "configurations:*Dll" }
        defines {
            "_GLFW_BUILD_DLL"
        }
    filter {}


    -- Custom Pre &// Post build Actions