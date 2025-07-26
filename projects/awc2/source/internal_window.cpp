#include "internal_window.hpp"
#include "awc2/window_types.hpp"
#include "internal_callback.hpp"
#include <GLFW/glfw3.h>
#include <util2/C/macro.h>
#include <util2/string.hpp>
#include "util2/aligned_malloc.hpp"
#include <cstdio>


namespace AWC2::internal {


static u8 __give_id_to_win = 0;


bool Window::create(
    u16 width, 
    u16 height, 
    WindowDescriptor winopt,
    GLFWwindow* win_share
) {
    /* OpenGL Context Hints */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_CLIENT_API,            GLFW_OPENGL_API);
    glfwWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS,    GLFW_LOSE_CONTEXT_ON_RESET); /* glGetGraphicsResetStatus() */
    /* Window Specific Hints*/
    glfwWindowHint(GLFW_VISIBLE, boolean(
        __scast(i32, winopt.createFlags & WindowCreationFlag::STARTUP_VISIBLE) 
    ));
    glfwWindowHint(GLFW_FOCUSED, boolean(
        __scast(i32, winopt.createFlags & WindowCreationFlag::STARTUP_FOCUSED)
    ));
    glfwWindowHint(GLFW_CENTER_CURSOR, boolean(
        __scast(i32, winopt.createFlags & WindowCreationFlag::STARTUP_CENTER_CURSOR) 
    ));
    glfwWindowHint(GLFW_RESIZABLE, boolean(
        __scast(i32, winopt.createFlags & WindowCreationFlag::RESIZABLE)
    ));
    glfwWindowHint(GLFW_DECORATED, boolean(
        __scast(i32, winopt.createFlags & WindowCreationFlag::BORDER)
    ));
    glfwWindowHint(GLFW_REFRESH_RATE, winopt.refreshRate == 0 
        ? GLFW_DONT_CARE : winopt.refreshRate
    );
    glfwWindowHint(GLFW_DEPTH_BITS,   (winopt.framebufferChannels >> 25) & 0b11111);
    glfwWindowHint(GLFW_STENCIL_BITS, (winopt.framebufferChannels >> 20) & 0b11111);
    glfwWindowHint(GLFW_RED_BITS,     (winopt.framebufferChannels >> 15) & 0b11111);
    glfwWindowHint(GLFW_GREEN_BITS,   (winopt.framebufferChannels >> 10) & 0b11111);
    glfwWindowHint(GLFW_BLUE_BITS,    (winopt.framebufferChannels >> 5 ) & 0b11111);
    glfwWindowHint(GLFW_ALPHA_BITS,   (winopt.framebufferChannels >> 0 ) & 0b11111);


    i32 size_s = DEFAULT32;
    char* strBuffer = nullptr;
	size_s    = std::snprintf(nullptr, 0, "Window %u", __give_id_to_win) + 1;
	strBuffer = __scast(char*, util2::aligned_malloc<8>(size_s));
	std::snprintf(strBuffer, size_s, "Window %u", __give_id_to_win);
    ++__give_id_to_win;
    
    
    m_data = {
        winopt,
        glfwCreateWindow(
            __scast(i32, width), 
            __scast(i32, height), 
            strBuffer, 
            nullptr, 
            win_share
        ),
        win_share,
        width,
        height
    };
    util2::aligned_free(strBuffer);
    return m_data.handle == nullptr;
}


void Window::destroy() {
    glfwDestroyWindow(m_data.handle);
    m_data.handle = __rcast(decltype(m_data.handle), DEFAULT64);
    m_data.parent_handle = nullptr;
    m_data.description = WindowDescriptor();
    m_data.width  = DEFAULT16;
    m_data.height = DEFAULT16;
    return;
}
void Window::setCurrent() const {
    glfwMakeContextCurrent(m_data.handle);
    return;
}
void Window::swapBuffers() const {
    glfwSwapBuffers(m_data.handle);
    return;
}
void Window::setVerticalSync(u8 val) const {
    glfwSwapInterval(__scast(i32, val));
    return;
}
void Window::setCallbacks(glfw_callback_table const* const glfw_callbacks) const {
    glfwSetWindowSizeCallback (m_data.handle, glfw_callbacks->framebuffer_size);
    glfwSetKeyCallback        (m_data.handle, glfw_callbacks->key);
    glfwSetWindowFocusCallback(m_data.handle, glfw_callbacks->window_focus);
    glfwSetCursorPosCallback  (m_data.handle, glfw_callbacks->cursor_position);
    glfwSetScrollCallback     (m_data.handle, glfw_callbacks->scroll_offset);
    glfwSetMouseButtonCallback(m_data.handle, glfw_callbacks->mouse_button);
    glfwSetWindowCloseCallback(m_data.handle, glfw_callbacks->close_window);
    glfwSetCursorEnterCallback(m_data.handle, glfw_callbacks->mouse_enter);
    glfwSetCharCallback       (m_data.handle, glfw_callbacks->text_input);
    /* 
        [NOTE]: This isn't context specific but still works when put here;
        [NOTE2]: 
                This still works since ImGui Contexts & GLFW windows 
                are tied together in a unified AWC2 Context 
        [TODO]: Move this to library init
    */
    glfwSetMonitorCallback(glfw_callbacks->monitor_events);
    return;
}
void Window::hide() const
{
    glfwHideWindow(m_data.handle);
    return;
}
void Window::setGLFWCloseFlag() const {
    glfwSetWindowShouldClose(m_data.handle, true);
    return;
}
bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_data.handle);
}


} // namespace AWC2::internal