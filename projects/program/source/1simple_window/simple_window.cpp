#include "simple_window.hpp"
#include <threads.h>
#include <util/marker2.hpp>
#include <util/vec2.hpp>
#include <awc2/C/awc2.h>
#include <util/marker2.hpp>


inline void custom_mousebutton_callback(AWC2User_callback_mousebutton_struct const* data)
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


using namespace util::math;


i32 simple_window()
{
    const struct timespec pause_sleep_duration{
        .tv_sec = 0,
        .tv_nsec = 6944444
    };
    const struct timespec slow_render_sleep_duration{
        .tv_sec = 0,
        .tv_nsec = 300 * 1000000
    };
    constexpr u8 slowRender{false};
    u8 alive {true};
    u8 paused{false};
    u8 contextid;
    vec2f simulationDimensions{1920, 1080};

    markstr("gpugems38_main begin");
    
    
    markstr("AWC2 init begin"); /* Init awc2 */
    awc2init();
    contextid = awc2createContext();
    AWC2ContextDescriptor ctxtinfo = {
        contextid,
        {0},
        __scast(u16, simulationDimensions.x),
        __scast(u16, simulationDimensions.y),
        AWC2WindowDescriptor{}
    };
    awc2initializeContext(&ctxtinfo);
    awc2setContextUserCallbackMouseButton(contextid, &custom_mousebutton_callback);
    markstr("AWC2 init end");


    markstr("Graphics Init Begin"); /* init graphics data */


    markstr("Graphics Init End  ");


    markstr("Main App Loop Begin"); /* Main App Loop */
    awc2setCurrentContext(contextid);
    while(alive) 
    {
        awc2newframe();
        awc2begin();
        if(paused) {
            thrd_sleep(&pause_sleep_duration, NULL);
        } else {
            if constexpr (slowRender) {
                thrd_sleep(&slow_render_sleep_duration, NULL);
            }
            // render(gfx, g_timing, simulationDimensions);
        }
        alive   = !awc2getContextStatus(contextid) && !awc2isKeyPressed(AWC2_KEYCODE_ESCAPE);
        paused ^= awc2isKeyPressed(AWC2_KEYCODE_P);
        awc2end();
    }
    markstr("Main App Loop End");


    /* return resources */
    markstr("Destroy AWC2 Context Begin");
    awc2destroyContext(contextid);
    awc2destroy();
    markstr("Destroy AWC2 Context End");
    markstr("gpugems38_main end  ");
    return 1;
}