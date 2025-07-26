project "awc2"
    systemversion "latest"
    warnings      "extra"
    rtti          "On"
    -- Project Structure
    files {
        "include/**.h",
        "include/**.hpp",
        "source/**.c",
        "source/**.cpp"
    }

    -- Specify Include Headers
    -- Other Project Includes Defined here...
    -- e.g IncludeProjectHeaders(...)
    includedirs { "include" }
    IncludeProjectHeaders("util2")
    IncludeProjectHeaders("glbinding")
    IncludeProjectHeaders("glbinding-aux")
    IncludeProjectHeaders("glfw34")
    IncludeProjectHeaders("imgui")

    
    -- Build Directories &// Structure
    SetupBuildDirectoriesForLibrary()

    -- Build Options
    buildoptions {}

    -- Linking Options
    LinkToStandardLibraries()
    LinkUtil2Library()
    LinkGLBindingLibraries()
    LinkGLFWLibrary()
    LinkImGuiLibrary()
    
    -- Macros
    filter { "configurations:*Lib" }
        defines { "AWC2_STATIC_DEFINE" }
    filter { "configurations:*Dll" }
        defines { "AWC2_EXPORTS" }
    filter {}

    -- Custom Pre &// Post build Actions