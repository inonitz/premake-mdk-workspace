project "glfw34"
    systemversion "latest"
    warnings      "extra"
    rtti          "On"
    SpecifyGlobalProjectCXXVersion()
    -- Project Structure
    files {
        "source/**.h",
        "source/**.c"
    }

    -- Specify Include Headers
    includedirs { "include" }

    -- Build Directories &// Structure
    SetupBuildDirectoriesForLibrary()

    -- Build Options
    filter { "files:**.h", "files:**.c" }
        cdialect "C11"
    filter { "files:**.hpp", "files:**.cpp" }
        cppdialect "C++17"
    filter {}


    -- Linking Options
    LinkToStandardLibraries()
    filter { "system:windows" }
        links { 
            "user32",
            "imm32",
            "gdi32",
            "shell32"
        }
    filter {}
    filter { "system:linux" }
        links { 
            "pthread",
            "dl", 
            "X11", 
        }
    filter {}


    -- Macros
    filter { "system:linux" }
        defines { 
            "_GLFW_X11"
        }
    filter { "system:windows" }
        defines { 
            "_GLFW_WIN32",
            "_CRT_SECURE_NO_WARNINGS" 
        }

    filter { "configurations:*Dll" }
        defines { "_GLFW_BUILD_DLL" }
    filter {}

    -- Custom Pre &// Post build Actions