project "imgui"
    systemversion "latest"
    warnings      "extra"
    rtti          "On"
    -- Project Structure
    files { 
        "include/**.h",
        "source/**.c",
        "include/**.hpp",
        "source/**.cpp"
    }
    -- Specify Include Headers
    -- Other Project Includes Defined here...
    -- e.g IncludeProjectHeaders(...)
    includedirs { "include" }
    IncludeProjectHeaders("glfw34")

    
    -- Build Directories &// Structure
    SetupBuildDirectoriesForLibrary()

    -- Build Options
    buildoptions {}

    -- Linking Options
    LinkToStandardLibraries()
    LinkGLFWLibrary()
    filter "system:windows"
        links { "imm32", "gdi32", "user32", "shell32" }
    
    filter "system:linux"
        links { "pthread", "dl", "X11", "Xrandr" }
    filter {}

    -- Macros
    filter { "configurations:*Lib" }
        defines { "IMGUI_STATIC_DEFINE" }
    filter { "configurations:*Dll" }
        defines { "IMGUI_EXPORTS" }
    filter {}


    -- Custom Pre &// Post build Actions