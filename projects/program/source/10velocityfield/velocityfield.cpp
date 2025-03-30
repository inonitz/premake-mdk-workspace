#include "velocityfield.hpp"
#include "backend10.hpp"
#include <threads.h>
#include <awc2/C/awc2.h>
#include <util/marker2.hpp>
#include <util/marker2.hpp>


i32 compute_shader_draw_full_velocity_field()
{
    const struct timespec pause_sleep_duration{
        .tv_sec = 0,
        .tv_nsec = 6944444
    };
    const struct timespec slow_render_sleep_duration{
        .tv_sec = 0,
        .tv_nsec = 300 * 1000000
    };
    u8 alive {true};
    u8 paused{false};


    markstr("compute_shader_draw_full_velocity_field begin");
    program::initializeLibrary();
    program::initializeGraphics();

    markstr("Main App Loop Begin"); /* Main App Loop */
    awc2setCurrentContext(program::getContextID());
    while(alive) 
    {
        program::getFrameTime().begin();
        awc2newframe();
        awc2begin();
        if(paused) {
            thrd_sleep(&pause_sleep_duration, NULL);
        } else {
            if(program::getSlowRenderFlag()) {
                thrd_sleep(&slow_render_sleep_duration, NULL);
            }
            program::render();
        }
        alive   = !awc2getContextStatus(program::getContextID()) && !awc2isKeyPressed(AWC2_KEYCODE_ESCAPE);
        paused ^= awc2isKeyPressed(AWC2_KEYCODE_P);
        if(awc2getCurrentContextWindowState() & AWC2_WINDOW_STATE_FLAG_MINIMIZED)
            paused = true;
        awc2end();
        program::getFrameTime().end();
    }
    markstr("Main App Loop End");

    /* return resources */
    program::destroyGraphics();
    program::destroyLibrary();
    markstr("compute_shader_draw_full_velocity_field end");
    return 1;
}