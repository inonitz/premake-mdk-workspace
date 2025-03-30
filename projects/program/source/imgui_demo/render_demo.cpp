#include "render_demo.hpp"
#include <threads.h>
#include <imgui/imgui.h>
#include <awc2/C/awc2.h>
#include <util/marker2.hpp>
#include <glbinding/gl/gl.h>


static inline void custom_mousebutton_callback(AWC2User_callback_mousebutton_struct const* data);


i32 render_imgui_demo_window()
{
    const struct timespec pause_sleep_duration{
        .tv_sec = 0,
        .tv_nsec = 6944444
    };
    u8 contextid{0};
    u8 alive{true}, paused{false};

    awc2init();
    contextid = awc2createContext();
    AWC2ContextDescriptor ctxtinfo = {
        contextid,
        {0},
        1920,
        1080,
        AWC2WindowDescriptor{}
    };
    awc2initializeContext(&ctxtinfo);
    awc2setContextUserCallbackMouseButton(contextid, &custom_mousebutton_callback);



    awc2setCurrentContext(contextid);
    while(alive) 
    {
        awc2newframe();
        awc2begin();

        gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);
        if(!paused) {
            ImGui::ShowDemoWindow();
        } else {
            thrd_sleep(&pause_sleep_duration, NULL);
        }
        alive   = !awc2getContextStatus(contextid) && !awc2isKeyPressed(AWC2_KEYCODE_ESCAPE);
        paused ^= awc2isKeyPressed(AWC2_KEYCODE_P);
        awc2end();
    }


    awc2destroyContext(contextid);
    awc2destroy();
    return 1;
}



static inline void custom_mousebutton_callback(AWC2User_callback_mousebutton_struct const* data)
{
    u8 state = (
        AWC2_MOUSEBUTTON_RIGHT == data->button &&
        AWC2_INPUTSTATE_PRESS  == data->action
    );
    if(state)
        awc2setCursorMode(AWC2_CURSORMODE_SCREEN_BOUND);
    else
        awc2setCursorMode(AWC2_CURSORMODE_NORMAL);


    return;
}