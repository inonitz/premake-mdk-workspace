project "glbinding-aux"
    systemversion "latest"
    warnings      "extra"
    rtti          "On"
    -- Project Structure
    files { 
        "include/**",
        "source/**.cpp" 
    }

    -- Specify Include Headers
    -- Other Project Includes Defined here...
    -- e.g IncludeProjectHeaders(...)
    includedirs { "include" }
    IncludeProjectHeaders("glbinding")

    -- Build Directories &// Structure
    SetupBuildDirectoriesForLibrary()

    -- Build Options
    buildoptions {}

    -- Linking Options
    LinkToStandardLibraries()
    LinkProjectLibrary("glbinding")
    filter { "system:linux" }
        links { "pthread" }
    filter {}
    
    -- Macros
    filter { "configurations:*Lib" }
        defines { "GLBINDING_STATIC_DEFINE", "GLBINDING_AUX_STATIC_DEFINE" }
    filter { "configurations:*Dll" }
        defines { "glbinding_aux_EXPORTS" }
    filter {}
    filter { "system:windows" }
        -- defines { "SYSTEM_WINDOWS" , "_CRT_SECURE_NO_WARNINGS" }
        defines { "SYSTEM_WINDOWS" }
    filter {}
    -- defines { 
    --     -- required for compilers using -std=c++20, -static-libstdc++ || clang_version < 19. 
    --     -- ==> https://github.com/msys2/MINGW-packages/issues/17730 [further read]
    --     "__GXX_TYPEINFO_EQUALITY_INLINE"
    -- }

    -- Custom Pre &// Post build Actions