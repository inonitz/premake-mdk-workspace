#include "multi_context_without_lib.hpp"
#include <iostream>
#include <glbinding/glbinding.h>
#include <glbinding/getProcAddress.h>
#include <glbinding/gl/gl.h>
#include <glbinding-aux/types_to_string.h>
#include <glbinding-aux/debug.h>
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>


using namespace gl;
using namespace glbinding;


struct PerWindow {
    unsigned int  id;
    bool          shouldExit;
    bool          hiddenWindow;
    ImGuiContext* imgui;
    GLFWwindow*   winptr;
};


void key_callback(GLFWwindow * window, int key, int /*scancode*/, int action, int /*mods*/)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        PerWindow* __userdata = reinterpret_cast<PerWindow*>(glfwGetWindowUserPointer(window));
        glfwSetWindowShouldClose(window, 1);
        __userdata->shouldExit = true;
        printf("%u close", __userdata->id);
    }
    return;
}


void framebuffer_callback(GLFWwindow* window, int width, int height)
{
    // glfwMakeContextCurrent(window);
    PerWindow* __userdata = reinterpret_cast<PerWindow*>(glfwGetWindowUserPointer(window));
    glbinding::useContext(__userdata->id);
    gl::glViewport(0, 0, width, height);
    return;
}


int awc2_wip_multi_context_without_encapsulation()
{
    glfwSetErrorCallback([](int errnum, const char* errmsg) {
        std::cerr << "GLFW ERROR " << errnum << ": " << errmsg << std::endl;
    });
    if (!glfwInit())
        return 1;
    IMGUI_CHECKVERSION();


    glfwDefaultWindowHints();
#ifdef SYSTEM_DARWIN
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

    int activeWindow = 0;
    int glver = 0;
    bool status;
    PerWindow winData[2] = {
        { 0, false, false,
            ImGui::CreateContext(),
            glfwCreateWindow(640, 480, "Window 0", nullptr, nullptr)
        }, 
        { 1, false, false,
            ImGui::CreateContext(),
            glfwCreateWindow(640, 480, "Window 1", nullptr, nullptr)
        }, 
    };
    if (!winData[0].winptr || !winData[1].winptr)
    {
        glfwTerminate();
        return -1;
    }


    for(auto& __data : winData) {
        printf("Window %u: %p, %p", 
            __data.id, 
            __data.imgui, 
            __data.winptr
        );
        glfwMakeContextCurrent(__data.winptr);
        glfwSetWindowUserPointer(__data.winptr, &__data  );
        glfwSetKeyCallback(__data.winptr, key_callback);
        glfwSetFramebufferSizeCallback(__data.winptr, framebuffer_callback);
        glbinding::initialize(__data.id, glbinding::getProcAddress, false, false);
        glbinding::aux::enableGetErrorCallback();
        ImGui::SetCurrentContext(__data.imgui);
        ImGui::StyleColorsDark();
        glver = ImGui_ImplGlfw_InitForOpenGL(__data.winptr, false);
        glver = glver && ImGui_ImplOpenGL3_Init("#version 460");
        if(!glver)
            return -1;    
    }

    status = !winData[0].hiddenWindow || !winData[1].hiddenWindow;
    while (status)
    {
        for(auto& __data : winData) {
            if(__data.hiddenWindow)
                continue;

            if(__data.shouldExit) {
                glfwHideWindow(__data.winptr);
                glfwMakeContextCurrent(nullptr);
                __data.hiddenWindow = true;
                __data.shouldExit = false;
                continue;
            }

            printf("Entered Draw Section for Context %u", __data.id);
            glfwMakeContextCurrent(__data.winptr);
            glbinding::useContext(__data.id);
            ImGui::SetCurrentContext(__data.imgui);
            /* Begin */
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            /* Actual Rendering Code */
            gl::glClearColor(1.0f, 0.0f, activeWindow / 255.0f, 0.0f);
            gl::glClear(gl::ClearBufferMask{GL_COLOR_BUFFER_BIT});
            /* End */
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(__data.winptr);
            ++activeWindow;
        }
        activeWindow %= 255; 


        glfwWaitEvents();
        status = false;
        for(auto& __data : winData) {
            status = status || !__data.hiddenWindow;
        }
    }


    for(auto& __data : winData) {
        ImGui::SetCurrentContext(__data.imgui);
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(__data.imgui);
        ImGui::SetCurrentContext(nullptr);
        glbinding::releaseContext(__data.id);
        glfwDestroyWindow(__data.winptr);
        __data.winptr = nullptr;
    }
    glfwTerminate();
    printf("Successful Exit");
    return 0;
}