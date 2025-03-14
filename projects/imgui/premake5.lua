project "imgui"
    systemversion "latest"
    warnings      "extra"
    rtti          "On"
    SpecifyGlobalProjectCXXVersion()
    -- Project Structure
    files { 
        "include/**.h",
        "source/**.c",
        "include/**.hpp",
        "source/**.cpp"
    }
    -- Specify Include Headers
    includedirs { 
        "include"
    }
    IncludeProjectHeaders("glfw-3.4")

    
    -- Build Directories &// Structure
    SetupBuildDirectoriesForLibrary()

    -- Build Options

    -- Linking Options
    LinkToStandardLibraries()
    LinkGLFWLibrary()
    filter "system:windows"
        links { 
            "imm32",
            "gdi32",
            "user32"
        }
    filter "system:linux"
        links { 
            "pthread",
            "dl", 
            "X11", 
            "Xrandr"
        }
    filter {}

    -- Macros
    filter { "configurations:*Lib" }
        defines { "IMGUI_STATIC_DEFINE" }
    filter { "configurations:*Dll" }
        defines { "IMGUI_EXPORTS" }
    filter {}


    -- Custom Pre &// Post build Actions