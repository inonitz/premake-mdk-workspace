LinkToStandardLibraries = function()
end


-- -- The reason for this mess: https://groups.google.com/g/llvm-dev/c/WA1vKn9zDtM
-- LinkToStandardLibraries = function()
--     -- Directly Taken from: https://learn.microsoft.com/en-us/cpp/c-runtime-library/crt-library-features?view=msvc-170
--     -- Static Debug:
--     -- Libraries: libucrtd.lib libvcruntimed.lib libcmtd.lib libcpmtd.lib
--     -- Macros: _DEBUG, _MT
--     filter { "system:windows", "action:gmake2", "configurations:DebugLib" }
--         buildoptions { "-fms-runtime-lib=static_dbg" }
--         defines { "_MT", "_DEBUG" }
--         linkoptions { "-Wl,/nodefaultlib,kernel32.lib,libcmtd.lib,libcpmtd.lib,libvcruntimed.lib,libucrtd.lib" }

--     -- Static Release: 
--     -- Libraries: libucrt.lib libvcruntime.lib libcmt.lib libcpmt.lib
--     -- Macros: _MT
--     filter { "system:windows", "action:gmake2", "configurations:ReleaseLib" }
--         buildoptions { "-fms-runtime-lib=static" }
--         defines { "_MT" }
--         linkoptions { "-Wl,/nodefaultlib,kernel32.lib,libcmt.lib,libcpmt.lib,libvcruntime.lib,libucrt.lib" }


--     -- Dll Debug: 
--     -- Libraries: ucrtd.lib vcruntimed.lib msvcrtd.lib msvcprtd.lib 
--     -- Macros: _DEBUG, _MT, _DLL
--     filter { "system:windows", "action:gmake2", "configurations:DebugDll" }
--         buildoptions { "-fms-runtime-lib=dll_dbg" }
--         defines { "_MT", "_DEBUG", "_DLL" }
--         linkoptions { "-Wl,/nodefaultlib,kernel32.lib,msvcrtd.lib,msvcprtd.lib,vcruntimed.lib,ucrtd.lib" }

--     -- Dll Release: 
--     -- Libraries: ucrt.lib vcruntime.lib msvcrt.lib msvcprt.lib
--     -- Macros: _MT, _DLL
--     filter { "system:windows", "action:gmake2", "configurations:ReleaseDll" }
--         buildoptions { "-fms-runtime-lib=dll" }
--         defines { "_MT", "_DLL" }
--         linkoptions { "-Wl,/nodefaultlib,kernel32.lib,msvcrt.lib,msvcprt.lib,vcruntime.lib,ucrt.lib" }

--     filter {}
-- end


-- Primarily for debugging
filter { "toolset:clang", "system:windows" }
    makesettings {
        "CC  = " .. '"' .. "clang.exe"   .. '"' .. " --verbose",
        -- "CXX = " .. '"' .. "clang++.exe" .. '"' .. " --verbose -ferror-limit=0 -fuse-ld=" .. '"' .. "C:/CTools/msys64/clang64/bin/lld-link" .. '"',
        "CXX = " .. '"' .. "clang++.exe" .. '"' .. " --verbose -ferror-limit=0 -fuse-ld=" .. '"' .. "C:/CTools/msys2/clang64/bin/ld.lld" .. '"',
        "LD  = " .. '"' .. "ld.lld.exe"  .. '"' .. " --verbose",
        "AR  = " .. '"' .. "llvm-ar.exe" .. '"' .. " v"
    }
filter {}
filter { "toolset:clang", "system:not windows" }
    makesettings {
        "CC  = " .. '"' .. "clang"   .. '"' .. " --verbose",
        "CXX = " .. '"' .. "clang++" .. '"' .. " --verbose -ferror-limit=0 -fuse-ld=" .. '"' .. "/usr/bin/ld.lld" .. '"',
        "LD  = " .. '"' .. "ld.lld"  .. '"' .. " --verbose",
        "AR  = " .. '"' .. "llvm-ar" .. '"' .. " v"
    }
-- filter {}
-- filter { "toolset:gcc" }
--     makesettings {
--         "CC  = " .. '"' .. "gcc" .. '"' .. " --verbose",
--         -- "CXX = " .. '"' .. "g++" .. '"' .. " --verbose -fmax-errors=0 -fuse-ld=" .. '"' .. "lld" .. '"',
--         "CXX = " .. '"' .. "g++" .. '"' .. " --verbose -fmax-errors=0",
--         "LD  = " .. '"' .. "lld"  .. '"' .. " --verbose",
--         "AR  = " .. '"' .. "ar"  .. '"' .. " v"
--     }
-- filter {}
-- filter { "system:windows", "action:gmake" }
-- filter { "system:windows", "action:vs2022" }
-- filter { "system:linux", "action:gmake" }


-- local function add_new_gcc_toolset (name, prefix)
--     local gcc                         = premake.tools.gcc
--     local new_toolset                 = {}  
--     new_toolset.getcflags             = gcc.getcflags
--     new_toolset.getcxxflags           = gcc.getcxxflags
--     new_toolset.getforceincludes      = gcc.getforceincludes
--     new_toolset.getldflags            = gcc.getldflags
--     new_toolset.getcppflags           = gcc.getcppflags
--     new_toolset.getdefines            = gcc.getdefines
--     new_toolset.getincludedirs        = gcc.getincludedirs
--     new_toolset.getLibraryDirectories = gcc.getLibraryDirectories
--     new_toolset.getlinks              = gcc.getlinks
--     new_toolset.getmakesettings       = gcc.getmakesettings
--     new_toolset.toolset_prefix        = prefix
--     function new_toolset.gettoolname (cfg, tool)  
--       if     tool == "cc" then
--         name = new_toolset.toolset_prefix .. "gcc"  
--       elseif tool == "cxx" then
--         name = new_toolset.toolset_prefix .. "g++"
--       elseif tool == "ar" then
--         name = new_toolset.toolset_prefix .. "ar"
--       end
--       return name
--     end  
--     premake.tools[name] = new_toolset
--   end
--   return add_new_gcc_toolset