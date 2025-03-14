project "awc2"
    systemversion "latest"
    warnings      "extra"
    rtti          "On"
    SpecifyGlobalProjectCXXVersion()
    -- Project Structure
    files {
        "include/**.h",
        "include/**.c",
        "include/**.hpp",
        "source/**.cpp"
    }

    -- Specify Include Headers
    includedirs { "include" }
    IncludeProjectHeaders("util")
    IncludeProjectHeaders("glfw-3.4")
    IncludeProjectHeaders("glbinding")
    IncludeProjectHeaders("glbinding-aux")
    IncludeProjectHeaders("imgui")

    
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
    LinkUtilLibrary()
    LinkGLFWLibrary()
    LinkGLBindingLibraries()
    LinkImGuiLibrary()
    
    -- Macros
    filter { "configurations:*Lib" }
        defines { "AWC2_STATIC_DEFINE" }
    filter { "configurations:*Dll" }
        defines { "AWC2_EXPORTS" }
    filter {}

    -- Custom Pre &// Post build Actions