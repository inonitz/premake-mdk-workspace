project "program"
    kind          "ConsoleApp"
    systemversion "latest"
    warnings      "extra"
    flags         "RelativeLinks"
    -- Project Structure
    files {
        "include/**.hpp",
        "include/**.h",

        "source/**.hpp",
        "source/**.cpp",

        "source/**.h",
        "source/**.c"
    }
    -- Specify Include Headers
    -- Other Project Includes Defined here...
    -- e.g IncludeProjectHeaders(...)
    includedirs { "include", "source" }
    IncludeProjectHeaders("util2")
    IncludeProjectHeaders("glfw34")
    IncludeProjectHeaders("glbinding")
    IncludeProjectHeaders("glbinding-aux")
    IncludeProjectHeaders("imgui")
    IncludeProjectHeaders("awc2")

    -- Build Directories &// Structure
    SetupBuildDirectoriesForExecutable()

    -- Build Options
    buildoptions {
        "-march=native"
    }

    -- Linking Options 
    LinkToStandardLibraries()
    -- Other Project Library Links Defined here...
    -- e.g LinkProjectALibrary(...)
    LinkUtil2Library()
    LinkGLBindingLibraries()
    LinkGLFWLibrary()
    LinkImGuiLibrary()
    LinkAWC2Library()
    filter "system:windows"
        links { "gdi32", "shell32", "pthread" }
    filter "system:linux" 
        links { "dl", "pthread" }
    filter {}


    -- Macros
    defines {}
    PreBuildCopyBuildTargetCompileCommandsToFolder()
    PostBuildCommmandsForExecutable()
    filter {}
