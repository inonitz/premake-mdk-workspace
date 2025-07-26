#ifndef __AWC2_INTERNAL_WINDOW_DEFINITION_HEADER__
#define __AWC2_INTERNAL_WINDOW_DEFINITION_HEADER__
#include "awc2/window_types.hpp"


typedef struct GLFWwindow GLFWwindow;


namespace AWC2::internal {


struct glfw_callback_table;


struct Window 
{
public:
    bool create(
        u16 width  = 720, 
        u16 height = 1080, 
        WindowDescriptor windowOptions = WindowDescriptor::getNullDescriptor(),
        GLFWwindow* multi_window_opengl_context_share = nullptr
    );
    void destroy();
    void setCurrent() const;
    void swapBuffers() const;
    void setVerticalSync(u8 val) const;
    void setCallbacks(glfw_callback_table const* const glfw_callbacks) const;
    void hide() const;
    void setGLFWCloseFlag() const;
    bool shouldClose() const;
    bool isMinimized() const {
        return (m_data.description.stateFlags & WindowStateFlag::MINIMIZED)
            == WindowStateFlag::MINIMIZED; 
    }
    bool sizeChanged() const {
        return (m_data.description.stateFlags & WindowStateFlag::SIZE_CHANGED)
            == WindowStateFlag::SIZE_CHANGED; 
    }
    bool isFocused() const {
        return (m_data.description.stateFlags & WindowStateFlag::FOCUSED)
            == WindowStateFlag::FOCUSED; 
    }
    WindowStateFlag getFlags() const { return m_data.description.stateFlags; }
    u32  getWidth()  const { return m_data.width;  }
    u32  getHeight() const { return m_data.height; }
    GLFWwindow* underlying_handle()        const { return m_data.handle;        }
    GLFWwindow* underlying_parent_handle() const { return m_data.parent_handle; }

public:
    struct alignsz(8) WindowData {
        WindowDescriptor description{};
        GLFWwindow* handle{nullptr};
        GLFWwindow* parent_handle{nullptr};
        u16 width{DEFAULT16}, height{DEFAULT16};
    };


    WindowData m_data;
};


} // namespace AWC2


#endif