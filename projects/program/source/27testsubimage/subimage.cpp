#include "subimage.hpp"
#include <threads.h>
#include <glbinding/gl/gl.h>
#include <util/marker2.hpp>
#include <util/vec2.hpp>
#include <util/aligned_malloc.hpp>
#include <awc2/C/awc2.h>
#include "awc2/C/context.h"
#include "awc2/C/input.h"
#include "gl/shader2.hpp"
#include "glbinding/gl/bitfield.h"
#include "glbinding/gl/functions.h"
#include "util/util.hpp"


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
    vec2i m_dims;
    u32   m_drawTexture;
    u32   m_interactTexture0;
    u32   m_interactTexture1;
    u32   m_fbo;
};


ShaderProgramV2 g_draw;
ShaderProgramV2 g_interact;
GLState state;
u8 contextid;
matrixView<vec4f> g_boundaryTextureData{};


static inline void render(GLState& gldata)
{
    static const vec4f defaultScreenColor = vec4f{1.0f, 1.0f, 1.0f, 1.0f}; 
    auto currentWindowSize = awc2getCurrentContextViewport();
    vec2i winSize = vec2i{
        currentWindowSize.x,
        currentWindowSize.y
    };
    auto mousePos = awc2getMousePosition();


    u8 status = 1;
    if(awc2isKeyPressed(AWC2_KEYCODE_R)) {
        g_draw.refreshFromFiles();
        g_draw.resizeLocalWorkGroup(0, { 1, 1, 1 });
        status = status && g_draw.compile();
        g_interact.refreshFromFiles();
        g_interact.resizeLocalWorkGroup(0, { 1, 1, 1 });
        status = status && g_interact.compile();
    }
    if(!status)
        return; /* couldn't recompile successfully, code needs recheck */



    gl::glClearNamedFramebufferfv( /* Clear Screen */
        gldata.m_fbo, 
        gl::GL_COLOR, 
        0, 
        defaultScreenColor.begin()
    );


    if(true) {
        g_interact.bind();
        g_interact.uniform1i("prevFrame", 0);
        g_interact.uniform1i("nextFrame", 1);
        g_interact.uniform2iv("ku_simdims",        gldata.m_dims.begin());
        g_interact.uniform2iv("ku_windims",        winSize.begin());
        g_interact.uniform1f("ku_splatterRadius",  0.03f);
        g_interact.uniform2fv("ku_mouseDragPos",   &mousePos.val[0]);
        g_interact.uniform1ui("ku_mousePressed",   awc2isMouseButtonPressed(AWC2_MOUSEBUTTON_RIGHT));
        gl::glBindTextureUnit(0, gldata.m_interactTexture0);
        gl::glBindImageTexture(1, gldata.m_interactTexture1, 0, false, 0, 
            gl::GL_WRITE_ONLY,
            gl::GL_RGBA32F
        );
        gl::glDispatchCompute(gldata.m_dims.x, gldata.m_dims.y, 1);
        gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);

        std::swap(gldata.m_interactTexture0, gldata.m_interactTexture1);
    }


    /* bind compute shader & bind texture, also fbo */
    g_draw.bind();
    g_draw.uniform1i("renderfrom", 0);
    g_draw.uniform1i("renderto", 1);
    gl::glBindTextureUnit(0, gldata.m_interactTexture0);
    gl::glBindImageTexture(1, gldata.m_drawTexture, 0, false, 0, 
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


i32 compute_user_interaction_with_subimage()
{
    static constexpr const char* computeShaderFilename[2] = {
        "projects/program/source/27testsubimage/0force.comp",
        "projects/program/source/27testsubimage/draw.comp"
    };
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



    markstr("compute_shader_render_to_screen begin");
    
    
    markstr("AWC2 init begin"); /* Init awc2 */
    awc2init();
    contextid = awc2createContext();
    AWC2ContextDescriptor ctxtinfo = {
        contextid,
        {0},
        __scast(u16, 1920),
        __scast(u16, 1080),
        AWC2WindowDescriptor{}
    };
    awc2WindowDescriptorDefault(&ctxtinfo.winDesc);
    awc2initializeContext(&ctxtinfo);
    awc2setContextUserCallbackMouseButton(contextid, &custom_mousebutton_callback);
    markstr("AWC2 init end");


    markstr("Graphics Init Begin"); /* init graphics data */
    state.m_dims = vec2i{1920, 1080};
    gl::glCreateTextures(gl::GL_TEXTURE_2D, 1, &state.m_drawTexture);
    gl::glCreateTextures(gl::GL_TEXTURE_2D, 1, &state.m_interactTexture0);
    gl::glCreateTextures(gl::GL_TEXTURE_2D, 1, &state.m_interactTexture1);
    gl::glCreateFramebuffers(1, &state.m_fbo);
    g_interact.createFrom({
        ShaderData{ computeShaderFilename[0], __scast(u32, gl::GL_COMPUTE_SHADER) }
    });
    g_interact.resizeLocalWorkGroup(0, { 1, 1, 1 });
    g_draw.createFrom({
        ShaderData{ computeShaderFilename[1], __scast(u32, gl::GL_COMPUTE_SHADER) }
    });
    g_draw.resizeLocalWorkGroup(0, { 1, 1, 1 });


    /* setup draw texture */
    markfmt("simdims %d %d", state.m_dims.x, state.m_dims.y);
    gl::glTextureParameteri(state.m_drawTexture, gl::GL_TEXTURE_WRAP_S, gl::GL_CLAMP_TO_EDGE);
    gl::glTextureParameteri(state.m_drawTexture, gl::GL_TEXTURE_WRAP_T, gl::GL_CLAMP_TO_EDGE);
    gl::glTextureParameteri(state.m_drawTexture, gl::GL_TEXTURE_MIN_FILTER, gl::GL_LINEAR);
    gl::glTextureParameteri(state.m_drawTexture, gl::GL_TEXTURE_MAG_FILTER, gl::GL_LINEAR);
    gl::glTextureStorage2D(state.m_drawTexture, 1, gl::GL_RGBA32F, state.m_dims.x, state.m_dims.y);
    gl::glTextureParameteri(state.m_interactTexture0, gl::GL_TEXTURE_WRAP_S, gl::GL_CLAMP_TO_EDGE);
    gl::glTextureParameteri(state.m_interactTexture0, gl::GL_TEXTURE_WRAP_T, gl::GL_CLAMP_TO_EDGE);
    gl::glTextureParameteri(state.m_interactTexture0, gl::GL_TEXTURE_MIN_FILTER, gl::GL_LINEAR);
    gl::glTextureParameteri(state.m_interactTexture0, gl::GL_TEXTURE_MAG_FILTER, gl::GL_LINEAR);
    // gl::glTextureStorage2D(state.m_interactTexture0, 1, gl::GL_RGBA32F, state.m_dims.x, state.m_dims.y);
    gl::glTextureParameteri(state.m_interactTexture1, gl::GL_TEXTURE_WRAP_S, gl::GL_CLAMP_TO_EDGE);
    gl::glTextureParameteri(state.m_interactTexture1, gl::GL_TEXTURE_WRAP_T, gl::GL_CLAMP_TO_EDGE);
    gl::glTextureParameteri(state.m_interactTexture1, gl::GL_TEXTURE_MIN_FILTER, gl::GL_LINEAR);
    gl::glTextureParameteri(state.m_interactTexture1, gl::GL_TEXTURE_MAG_FILTER, gl::GL_LINEAR);
    // gl::glTextureStorage2D(state.m_interactTexture1, 1, gl::GL_RGBA32F, state.m_dims.x, state.m_dims.y);
    
    
    vec4f* boundaries = __rcast(vec4f*, util::aligned_malloc<sizeof(vec4f)>(1920 * 1080 * sizeof(vec4f)));
    markfmt("address is %lX", boundaries);
    util::__memset(boundaries, 1920 * 1080, vec4f{0.0f});
    for(i32 i = 0; i < state.m_dims.y; ++i) {
        boundaries[state.m_dims.y * (state.m_dims.x - 10) + i] = vec4f{1};
    }


    gl::glBindTexture(gl::GL_TEXTURE_2D, state.m_interactTexture0);
    gl::glTexImage2D(
        gl::GL_TEXTURE_2D, 
        0, 
        gl::GL_RGBA32F, 
        state.m_dims.x, 
        state.m_dims.y, 
        0, 
        gl::GL_RGBA, 
        gl::GL_FLOAT, 
        boundaries
    );
    gl::glBindTexture(gl::GL_TEXTURE_2D, state.m_interactTexture1);
    gl::glTexImage2D(
        gl::GL_TEXTURE_2D, 
        0, 
        gl::GL_RGBA32F, 
        state.m_dims.x, 
        state.m_dims.y, 
        0, 
        gl::GL_RGBA, 
        gl::GL_FLOAT, 
        boundaries
    );
    gl::glBindTexture(gl::GL_TEXTURE_2D, 0);



    // gl::glTextureSubImage2D(state.m_interactTexture0, 0, 
    //     0, 0, state.m_dims.x, state.m_dims.y,
    //     gl::GL_RGBA, gl::GL_FLOAT, 
    //     boundaries
    // );
    // gl::glTextureSubImage2D(state.m_interactTexture1, 0, 
    //     0, 0, state.m_dims.x, state.m_dims.y,
    //     gl::GL_RGBA, gl::GL_FLOAT, 
    //     boundaries
    // );
    // gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
    util::aligned_free(boundaries);


    gl::glNamedFramebufferTexture(state.m_fbo, gl::GL_COLOR_ATTACHMENT0, state.m_drawTexture, 0);
    ifcrash( gl::glCheckNamedFramebufferStatus(state.m_fbo, gl::GL_FRAMEBUFFER) 
        != gl::GL_FRAMEBUFFER_COMPLETE 
    );
    ifcrashstr(!g_interact.compile(), "Unsuccessful shader compile");
    ifcrashstr(!g_draw.compile(), "Unsuccessful shader compile");
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
    gl::glDeleteTextures(1, &state.m_drawTexture);
    gl::glDeleteTextures(1, &state.m_interactTexture0);
    gl::glDeleteTextures(1, &state.m_interactTexture1);
    gl::glDeleteFramebuffers(1, &state.m_fbo);
    g_draw.destroy();
    g_interact.destroy();
    markstr("Graphics Destroy End");

    markstr("AWC2 Destroy Begin");
    awc2destroyContext(contextid);
    awc2destroy();
    markstr("AWC2 Destroy End");



    markstr("compute_shader_render_to_screen end  ");
    return 1;
}