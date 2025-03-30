#include "internal_state.hpp"
#include "internal_callback.hpp"
#include "internal_instance.hpp"
#include "internal_event.hpp"
#include <util/ifcrash.hpp>
#include <util/marker2.hpp>
#include <util/util.hpp>

#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <glbinding-aux/debug.h>
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>


namespace AWC2::internal {


void unbindActiveContext() {
    markstr("unbindActiveContext() begin");
    ImGui::SetCurrentContext(nullptr);
    glbinding::useContext(0);
    glfwMakeContextCurrent(nullptr);
    markstr("unbindActiveContext() end");
    return;
}



void AWC2ContextData::create(
    u8 __internal_context_id,
    u16 __win_width, 
    u16 __win_height, 
    WindowDescriptor const& __win_desc
) {
    u64 win_desc_bits;
    WindowDescriptor win_desc = __win_desc;
    const glfw_callback_table default_callbacks{};


    /* GLFW Window */
    util::__memcpy(&win_desc_bits, __rcast(u64*, &win_desc), 1);
    bool not_good = m_window.create(
        __win_width, 
        __win_height,
        win_desc_bits,
        nullptr
    );
    if(not_good) {
        const char* msg;
        u32 what_happened = glfwGetError(&msg);
        ifcrashfmt(true, "AWC2ContextData::create(...) ==> Couldn't Create underlying GLFW Window\n  => Error #%u: %s", what_happened, msg);
    }
    m_window.setCallbacks(&default_callbacks);
    glfwSetWindowUserPointer(m_window.underlying_handle(), &m_id);
    
    /* Reset IO */
    m_io.reset();

    /* ImGui */
    m_imgui = ImGui::CreateContext();
    /*
        Internal ID - Needs to be in context memory for glfwGet/SetUserPointer 
        (!!!) Allocation_Index + 1 === id (!!!)
    */
    m_id = __internal_context_id;
    m_FlagInit   = true;
    m_FlagClosed = false;
    return;
}


void AWC2ContextData::initialize() 
{
    i32 glver = 0;


    /* User Defined Event table */
    util::__memset<u64>(m_event_table.pointers, 
        __carraysize(m_event_table.pointers) - __carraysize(m_event_table.reserved0), 
        __rcast(u64, &user_callback_func_noop)
    );
    glver = 0;


    /* Bind Current GLFW Context */
    m_window.setCurrent();
    
    /* OpenGL Context (Can only be initialized when creating its context ... ) */
    glbinding::initialize(m_id, glfwGetProcAddress, false, false);
    glbinding::aux::enableGetErrorCallback();

    /* Can only happen after an opengl context is created */
    bool useVsync = boolean(__scast(i32, 
        m_window.m_data.description.createFlags 
        & 
        WindowCreationFlag::USE_VSYNC
    ));
    m_window.setVerticalSync(useVsync);
    
    /* ImGui */
    ImGui::SetCurrentContext(__rcast(ImGuiContext*, m_imgui));
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    // io.ConfigViewportsNoAutoMerge = true;
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigViewportsNoTaskBarIcon = true;
    
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::StyleColorsDark();
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;


    glver = ImGui_ImplGlfw_InitForOpenGL(m_window.underlying_handle(), false);
    glver = glver && ImGui_ImplOpenGL3_Init("#version 450");
    ifcrashstr_debug(!glver, "Failed to initialize AWC2 context\n");

    m_FlagInit = true;
    return;
}


void AWC2ContextData::close()
{
    m_window.setGLFWCloseFlag();
    m_window.hide();
    m_FlagClosed = true;
    return;
}


void AWC2ContextData::destroy()
{
    setCurrent();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    ImGui::SetCurrentContext(nullptr);
    glbinding::releaseContext(m_id);
    glfwMakeContextCurrent(nullptr);
    m_window.destroy();

    /* Reset Variable Members */
    util::__memset(reserved, __carraysize(reserved));
    m_FlagClosed = DEFAULT8;
    m_FlagInit   = DEFAULT8;
    m_id         = DEFAULT8;
    m_imgui      = DEFAULTVOIDPTR;
    m_io.reset();
    util::__memset(
        m_event_table.pointers, 
        __carraysize(m_event_table.pointers), 
        DEFAULT64
    );
    return;
}


void AWC2ContextData::setCurrent() const
{
    m_window.setCurrent();
    glbinding::useContext(m_id);
    ImGui::SetCurrentContext(__rcast(ImGuiContext*, m_imgui));
    return;
}


void AWC2ContextData::begin()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    return;
}


void AWC2ContextData::end()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


    /* Temporary until I fully understand how to incorporate this without excess code */
    GLFWwindow* backup_current_context = glfwGetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(backup_current_context);


    m_window.swapBuffers();
    m_io.reset();
    return;
}


} // namespace AWC2::internal
