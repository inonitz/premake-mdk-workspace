#include "fullsim.hpp"
#include "backend11.hpp"
#include <threads.h>
#include <awc2/C/awc2.h>
#include <util/marker2.hpp>
#include <util/marker2.hpp>


i32 compute_shader_draw_full_simulation_field()
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
    u8 paused{false}, prevPause{false};


    markstr("compute_shader_draw_full_simulation_field begin");
    program2::initializeLibrary();
    program2::initializeGraphics();

    markstr("Main App Loop Begin"); /* Main App Loop */
    awc2setCurrentContext(program2::getContextID());
    while(alive) 
    {
        program2::getFrameTime().begin();
        awc2newframe();
        awc2begin();
        if(paused) {
            thrd_sleep(&pause_sleep_duration, NULL);
        } else {
            if(program2::getSlowRenderFlag()) {
                thrd_sleep(&slow_render_sleep_duration, NULL);
            }
            program2::render();
        }
        alive   = !awc2getContextStatus(program2::getContextID()) && !awc2isKeyPressed(AWC2_KEYCODE_ESCAPE);
        
        paused ^= awc2isKeyPressed(AWC2_KEYCODE_P);
        if(awc2getCurrentContextWindowState() & AWC2_WINDOW_STATE_FLAG_MINIMIZED)
            paused = true;
        awc2end();
        program2::getFrameTime().end();
    }
    markstr("Main App Loop End");

    /* return resources */
    program2::destroyGraphics();
    program2::destroyLibrary();
    markstr("compute_shader_draw_full_simulation_field end");
    return 1;
}