include "searchlocal.lua"
require(".vscode/export-compile-comms")
require(".vscode/premake-ninja")


WORKSPACE_NAME = "DefaultWorkspace"
PROJECT_LIST   = {
    "projects/util2",
    "projects/glbinding",
    "projects/glbinding-aux",
    "projects/glfw34",
    "projects/imgui",
    "projects/awc2",
    "projects/program"
}
START_PROJECT = "program"


include "toolchain.lua"
include "workspace.lua"
include "dir.lua"
for _, path in ipairs(PROJECT_LIST) do
    include(path .. "/premake5.lua")
end
include "cmd.lua"