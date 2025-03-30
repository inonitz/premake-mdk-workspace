#include "compute_screen.hpp"
#include <threads.h>
#include <glbinding/gl/gl.h>
#include <util/marker2.hpp>
#include <util/vec2.hpp>
#include <awc2/C/awc2.h>
#include <util/marker2.hpp>
#include "awc2/C/context.h"
#include "gl/shader2.hpp"


using namespace util::math;


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


struct GLState {
    vec2u m_dims;
    u32   m_texture;
    u32   m_fbo;
    ShaderProgramV2 compute;
};  


static inline void render(GLState& gldata)
{
    static const vec4f defaultScreenColor = vec4f{1.0f, 1.0f, 1.0f, 1.0f}; 
    auto currentWindowSize = awc2getCurrentContextViewport();

    u8 status = 1;
    if(awc2isKeyPressed(AWC2_KEYCODE_R)) {
        gldata.compute.refreshFromFiles();
        gldata.compute.resizeLocalWorkGroup(0, 1, 1, 1);
        status = gldata.compute.compile();
    }
    if(!status)
        return; /* couldn't recompile successfully, code needs recheck */



    gl::glClearNamedFramebufferfv( /* Clear Screen */
        gldata.m_fbo, 
        gl::GL_COLOR, 
        0, 
        defaultScreenColor.begin()
    );


    /* bind compute shader & bind texture, also fbo */
    gldata.compute.bind();
    gldata.compute.uniform1i("renderto", 1);
    gl::glBindImageTexture(1, gldata.m_texture, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
    gl::glDispatchCompute(gldata.m_dims.x, gldata.m_dims.y, 1);


    /* Draw Call */
    gl::glBlitNamedFramebuffer(gldata.m_fbo, 0, 
        0, 0, gldata.m_dims.x,     gldata.m_dims.y, 
        0, 0, currentWindowSize.x, currentWindowSize.y,
        gl::GL_COLOR_BUFFER_BIT, 
        gl::GL_LINEAR
    );
}


i32 compute_shader_render_to_screen()
{
    static constexpr const char* computeShaderFilename = "projects/program/source/2compute_screen/compute_screen.comp";
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
    vec2u simulationDimensions{1920, 1080};


    markstr("compute_shader_render_to_screen begin");
    
    
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
    awc2WindowDescriptorDefault(&ctxtinfo.winDesc);
    awc2initializeContext(&ctxtinfo);
    awc2setContextUserCallbackMouseButton(contextid, &custom_mousebutton_callback);
    markstr("AWC2 init end");


    markstr("Graphics Init Begin"); /* init graphics data */
    GLState state;
    state.m_dims = simulationDimensions;
    gl::glCreateTextures(gl::GL_TEXTURE_2D, 1, &state.m_texture);
    gl::glCreateFramebuffers(1, &state.m_fbo);
    state.compute.createFrom({
        ShaderData{ computeShaderFilename, __scast(u32, gl::GL_COMPUTE_SHADER) }
    });
    state.compute.resizeLocalWorkGroup(0, 1, 1, 1);


    /* setup draw texture */
    gl::glTextureParameteri(state.m_texture, gl::GL_TEXTURE_WRAP_S, gl::GL_CLAMP_TO_EDGE);
    gl::glTextureParameteri(state.m_texture, gl::GL_TEXTURE_WRAP_T, gl::GL_CLAMP_TO_EDGE);
    gl::glTextureParameteri(state.m_texture, gl::GL_TEXTURE_MIN_FILTER, gl::GL_LINEAR);
    gl::glTextureParameteri(state.m_texture, gl::GL_TEXTURE_MAG_FILTER, gl::GL_LINEAR);
    gl::glTextureStorage2D(state.m_texture, 1, gl::GL_RGBA32F, simulationDimensions.x, simulationDimensions.y);
    gl::glNamedFramebufferTexture(state.m_fbo, gl::GL_COLOR_ATTACHMENT0, state.m_texture, 0);
    ifcrash( gl::glCheckNamedFramebufferStatus(state.m_fbo, gl::GL_FRAMEBUFFER) 
        != gl::GL_FRAMEBUFFER_COMPLETE 
    );
    ifcrashstr(!state.compute.compile(), "Unsuccessful shader compile");


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
            render(state);
        }
        alive   = !awc2getContextStatus(contextid) && !awc2isKeyPressed(AWC2_KEYCODE_ESCAPE);
        paused ^= awc2isKeyPressed(AWC2_KEYCODE_P);
        awc2end();
    }
    markstr("Main App Loop End");


    /* return resources */
    markstr("Graphics Destroy Begin");
    gl::glDeleteTextures(1, &state.m_texture);
    gl::glDeleteFramebuffers(1, &state.m_fbo);
    state.compute.destroy();
    markstr("Graphics Destroy End");

    markstr("AWC2 Destroy Begin");
    awc2destroyContext(contextid);
    awc2destroy();
    markstr("AWC2 Destroy End");



    markstr("compute_shader_render_to_screen end  ");
    return 1;
}