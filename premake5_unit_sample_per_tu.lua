-- This is taken from the GoogleTest-Mock Branch: https://github.com/inonitz/premake5-workspace-template/tree/GoogleTest-Mock/projects/sample


-- Having a function to generate a project for each Test file is cumbersome,
-- Stupid, and frankly insane. How the hell am I supposed to figure out
-- that appearently there are inner-dependencies between these projects,
-- unless I manually tinker with each one until everything is compiles?
-- What the hell?
-- https://github.com/premake/premake-core/issues/998
defineTestProject = function(name,file_array)
project(name)
    kind          "ConsoleApp"
    systemversion "latest"
    warnings      "extra"

    for _, tu_file in ipairs(file_array) do
        files {
            tu_file
        }
    end
    -- Specify Include Headers
    -- Other Project Includes Defined here...
    -- e.g IncludeProjectHeaders(...)
    includedirs { 
        "include", 
        "source",
        _WORKING_DIR .. "/projects/googletest/googletest"
    }
    IncludeProjectHeaders("googletest/googletest")
    IncludeProjectHeaders("googletest/googlemock")


    -- Build Directories &|| Structure
    SetupBuildDirectoriesForExecutable()

    -- Build Options
    buildoptions {
        "-march=native"
    }

    -- Linking Options 
    LinkToStandardLibraries()
    -- Other Project Library Links Defined here...
    -- e.g LinkProjectALibrary(...)
    LinkGoogleTestLibrary()

    filter "system:windows"
        links { "shell32", "pthread" }
    filter "system:linux" 
        links { "dl", "pthread" }
    filter {}

    -- Macros
    defines {}

    -- Post build commands
    PreBuildCopyBuildTargetCompileCommandsToFolder()
    PostBuildCommmandsForExecutable()
end


-- Thanks to: https://stackoverflow.com/questions/1426954/split-string-in-lua
function splitstr_by_char(inputstr, sep)
    if sep == nil then
        sep = "%s"
    end
    local t = {}
    for str in string.gmatch(inputstr, "([^"..sep.."]+)") do
        table.insert(t, str)
    end
    return t
end

function getPath(str)
    return str:match("(.*[/\\])")
end



-- local realpath, err = os.realpath(fname)
-- if err then
--     printf("Error generating premake5 Test project - %s", err)
-- end
-- local test_files = os.matchfiles(_WORKING_DIR .. "/projects/sample/source/test/**.cc")  -- recursive match
-- for _, fname in ipairs(test_files) do
--     local path_noext = string.match(fname, "(.+)%..+$")
--     local all_words = splitstr_by_char(path_noext, "/")
--     -- print(path_noext)
--     -- print(all_words[#all_words])
--     defineTestProject(fname, getPath(fname), all_words[#all_words])
-- end


defineTestProject("sample1", { 
    _WORKING_DIR .. "/projects/sample/source/"             .. "sample1_unittest.cc",
    _WORKING_DIR .. "/projects/googletest/googletest/src/" .. "gtest_main.cc",
    _WORKING_DIR .. "/projects/sample/source/"             .. "sample1.cc"
})
defineTestProject("sample2", { 
    _WORKING_DIR .. "/projects/sample/source/"             .. "sample2_unittest.cc",
    _WORKING_DIR .. "/projects/googletest/googletest/src/" .. "gtest_main.cc",
    _WORKING_DIR .. "/projects/sample/source/"             .. "sample2.cc"
})
defineTestProject("sample3", { 
    _WORKING_DIR .. "/projects/sample/source/"             .. "sample3_unittest.cc",
    _WORKING_DIR .. "/projects/googletest/googletest/src/" .. "gtest_main.cc",
})
defineTestProject("sample4", { 
    _WORKING_DIR .. "/projects/sample/source/"             .. "sample4_unittest.cc", 
    _WORKING_DIR .. "/projects/googletest/googletest/src/" .. "gtest_main.cc",
    _WORKING_DIR .. "/projects/sample/source/"             .. "sample4.cc"
})
defineTestProject("sample5", { 
    _WORKING_DIR .. "/projects/sample/source/"             .. "sample5_unittest.cc", 
    _WORKING_DIR .. "/projects/googletest/googletest/src/" .. "gtest_main.cc",
    _WORKING_DIR .. "/projects/sample/source/"             .. "sample1.cc"
})
defineTestProject("sample6", { 
    _WORKING_DIR .. "/projects/sample/source/"             .. "sample6_unittest.cc", 
    _WORKING_DIR .. "/projects/googletest/googletest/src/" .. "gtest_main.cc"
})
defineTestProject("sample7", { 
    _WORKING_DIR .. "/projects/sample/source/"             .. "sample7_unittest.cc", 
    _WORKING_DIR .. "/projects/googletest/googletest/src/" .. "gtest_main.cc"
})
defineTestProject("sample8", { 
    _WORKING_DIR .. "/projects/sample/source/"             .. "sample8_unittest.cc", 
    _WORKING_DIR .. "/projects/googletest/googletest/src/" .. "gtest_main.cc"
})
defineTestProject("sample9", { 
    _WORKING_DIR .. "/projects/sample/source/"             .. "sample9_unittest.cc"
    -- _WORKING_DIR .. "/projects/googletest/googletest/src/" .. "gtest.cc"
})
defineTestProject("sample10", { 
    _WORKING_DIR .. "/projects/sample/source/"             .. "sample10_unittest.cc"
    -- _WORKING_DIR .. "/projects/googletest/googletest/src/" .. "gtest.cc" 
})
