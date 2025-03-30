#include "compute_buffer_mouse_diffusion.hpp"
#include <threads.h>
#include <glbinding/gl/gl.h>
#include <imgui/imgui.h>
#include <util/marker2.hpp>
#include <util/vec2.hpp>
#include <util/random.hpp>
#include <awc2/C/awc2.h>
#include <util/marker2.hpp>
#include "awc2/C/input.h"
#include "gl/shader2.hpp"
#include "glbinding/gl/enum.h"
#include "glbinding/gl/functions.h"
#include "util/macro.h"


using namespace util::math;


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


// static inline void initializeVectorFields(vec2u const& dims, std::vector<vec4f>& field) {
//     mark();
//     field.resize(dims.x * dims.y);
//     for(u64 i = 0; i < field.size(); ++i) {
//         f32 rand = random32f();
//         field[i] = vec4f{
//             0.0f, 0.0f, rand, 1.0f
//         };
//     }
//     mark();
//     return;
// }


#if defined __linux__
    static constexpr const char* computeShaderFilename[3] = {
        "projects/program/source/7compute_buffer_mouse_diffusion/compute_buffer_mouse_diffusion1.comp",
        "projects/program/source/7compute_buffer_mouse_diffusion/compute_buffer_mouse_diffusion2.comp",
        "projects/program/source/7compute_buffer_mouse_diffusion/compute_buffer_mouse_diffusion3.comp"
    };
#elif defined _WIN32
    static constexpr const char* computeShaderFilename[3] = {
        "C:/CTools/Projects/main/projects/program/source/7compute_buffer_mouse_diffusion/compute_buffer_mouse_diffusion1.comp",
        "C:/CTools/Projects/main/projects/program/source/7compute_buffer_mouse_diffusion/compute_buffer_mouse_diffusion2.comp",
        "C:/CTools/Projects/main/projects/program/source/7compute_buffer_mouse_diffusion/compute_buffer_mouse_diffusion3.comp"
    };
#endif
static std::vector<vec4f> g_initialField;
static vec2u              g_dims{1920, 1080};
static f32                g_dt        = 0.01f;
static f32                g_viscosity = 0.2f;
static u32                g_maximumJacobiIterations = 60;
static u32                g_texture[4];
static u32                g_fbo;
static ShaderProgramV2    g_compute[__carraysize(computeShaderFilename)];
static vec2f              g_mouseDragForce;
static vec2f              g_mouseDragPosition;
static f32                g_splatterRadius{5};
static vec4f              g_splatterColor{1.0f, 1.0f, 1.0f, 1.0f};
static bool               g_mousePressed{false};
static u32                g_frameCounter{0};


static ShaderProgramV2& gr_computeInteractive = g_compute[0];
static ShaderProgramV2& gr_computeDiffusion   = g_compute[1];
static ShaderProgramV2& gr_computeRenderToTex = g_compute[2];


static void render_imgui_interface()
{
    // if(awc2isMouseMoving() && awc2isMouseButtonPressed(AWC2_MOUSEBUTTON_LEFT)) {
    if(awc2isMouseButtonPressed(AWC2_MOUSEBUTTON_LEFT)) {
        // mark();
        AWC2CursorPosition delta = awc2getMousePositionDelta();
        AWC2CursorPosition pos = awc2getMousePosition();

        g_mouseDragForce    = vec2f{ delta.x, delta.y };
        g_mouseDragPosition = vec2f{ pos.x, pos.y };
        g_mousePressed      = true;
    }

    char* dragForceStr    = g_mouseDragForce.to_string();
    char* mouseDragPosStr = g_mouseDragPosition.to_string();

    ImGui::Begin("Simulation Parameters");
    ImGui::Text("\
Mouse Dragging Force %s\n\
Mouse Position       %s\n\
Mouse was Pressed    %u\n",
    g_mouseDragForce.to_string(), 
    mouseDragPosStr,
    g_mousePressed
    );
    ImGui::DragFloat("splatterRadius", &g_splatterRadius, 0.1f, 0.1, 2.0f);
    ImGui::ColorEdit4("splatterColor", g_splatterColor.begin());
    ImGui::DragFloat("Delta Time", &g_dt, 0.01f, 0.001, 0.2);
    ImGui::DragFloat("Viscosity Coefficient", &g_viscosity, 0.05f, 0.01f, 1.0f);
    ImGui::End();


    free(dragForceStr);
    free(mouseDragPosStr);
    return;
}


static inline void render()
{
    static const vec4f defaultScreenColor = vec4f{1.0f, 1.0f, 1.0f, 1.0f}; 
    vec2i cvt = vec2i{g_dims.x, g_dims.y};
    auto  currentWindowSize = awc2getCurrentContextViewport();
    u8    status = 1;


    if(awc2isKeyPressed(AWC2_KEYCODE_R)) {
        for(auto& comp : g_compute) {
            comp.refreshFromFiles();
            comp.resizeLocalWorkGroup(0, 1, 1, 1 );
            status = status && comp.compile();
        }
    }
    if(!status)
        return; /* couldn't recompile successfully, code needs recheck */



    gl::glClearNamedFramebufferfv( /* Clear Screen */
        g_fbo, 
        gl::GL_COLOR, 
        0, 
        defaultScreenColor.begin()
    );


    render_imgui_interface();


    u32 previousIteration, nextIteration, temporaryTexture, drawTexture;
    u32 textureInput, textureOutput;
    if(!(g_frameCounter % 2)) {
        previousIteration = g_texture[0];
        nextIteration     = g_texture[1];
    } else {
        previousIteration = g_texture[1];
        nextIteration     = g_texture[0];
    }
    temporaryTexture = g_texture[3];
    drawTexture      = g_texture[2];


    /* write some shit to a texture */
    gr_computeInteractive.bind();
    gr_computeInteractive.uniform1i("ping", 0);
    gr_computeInteractive.uniform1i("pong", 1);
    gr_computeInteractive.uniform2iv("ku_simdims", cvt.begin());
    gr_computeInteractive.uniform2fv("ku_mouseDragForce", g_mouseDragForce.begin()   );
    gr_computeInteractive.uniform2fv("ku_mouseDragPos",   g_mouseDragPosition.begin());
    gr_computeInteractive.uniform1f("ku_splatterRadius",  g_splatterRadius); 
    gr_computeInteractive.uniform4fv("ku_splatterColor",  g_splatterColor.begin()); 
    gr_computeInteractive.uniform1ui("ku_mousePressed",   g_mousePressed);
    gl::glBindTextureUnit(0, previousIteration);
    gl::glBindImageTexture(1, temporaryTexture, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    gl::glDispatchCompute(g_dims.x, g_dims.y, 1);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);


    /* apply diffusion to whatever we got previously, write it to some other texture */
    gr_computeDiffusion.bind();
    gr_computeDiffusion.uniform1i("initialField", 0);
    gr_computeDiffusion.uniform1i("outputField",  1);
    gr_computeDiffusion.uniform1f("ku_dt",        g_dt);
    gr_computeDiffusion.uniform1f("ku_viscosity", g_viscosity);
    gr_computeDiffusion.uniform2iv("ku_simdims", cvt.begin());

    textureInput  = temporaryTexture;
    textureOutput = nextIteration;
    for(u32 i = 0; i < g_maximumJacobiIterations; ++i) {
        gl::glBindTextureUnit(0, textureInput);
        gl::glBindImageTexture(1, textureOutput, 0, false, 0, 
            gl::GL_WRITE_ONLY, 
            gl::GL_RGBA32F
        );
        gl::glDispatchCompute(g_dims.x, g_dims.y, 1);


        u32 tmp = textureInput;
        textureInput  = textureOutput;
        textureOutput = tmp;
        gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
    }


    /* literally just show that other texture on the screen */
    gr_computeRenderToTex.bind();
    gr_computeRenderToTex.uniform1i("fieldSampler", 0);
    gr_computeRenderToTex.uniform1i("screentexture", 1);
    gr_computeRenderToTex.uniform2iv("ku_simdims", cvt.begin());
    gl::glBindTextureUnit(0, nextIteration);
    gl::glBindImageTexture(1, drawTexture, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    gl::glDispatchCompute(g_dims.x, g_dims.y, 1);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);


    /* Draw Call */
    // gl::glNamedFramebufferTexture(g_fbo, gl::GL_COLOR_ATTACHMENT0, g_texture[1], 0);
    gl::glBlitNamedFramebuffer(g_fbo, 0, 
        0, 0, g_dims.x,     g_dims.y, 
        0, 0, currentWindowSize.x, currentWindowSize.y,
        gl::GL_COLOR_BUFFER_BIT, 
        gl::GL_NEAREST
    );
    


    g_mousePressed = false;
    ++g_frameCounter;
    return;
}


i32 compute_buffer_mouse_but_with_diffusion()
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


    markstr("compute_shader_render_buffer_to_screen begin");
    
    
    markstr("AWC2 init begin"); /* Init awc2 */
    awc2init();
    contextid = awc2createContext();
    AWC2ContextDescriptor ctxtinfo = {
        contextid,
        {0},
        __scast(u16, g_dims.x),
        __scast(u16, g_dims.y),
        AWC2WindowDescriptor{}
    };
    awc2initializeContext(&ctxtinfo);
    awc2setContextUserCallbackMouseButton(contextid, &custom_mousebutton_callback);
    markstr("AWC2 init end");


    markstr("Graphics Init Begin"); /* init graphics data */
    gl::glCreateTextures(gl::GL_TEXTURE_2D, __carraysize(g_texture), &g_texture[0]);
    gl::glCreateFramebuffers(1, &g_fbo);
    for(uint32_t i = 0; alive && i < __carraysize(computeShaderFilename); ++i) {
        g_compute[i].createFrom({
            ShaderData{ computeShaderFilename[i], __scast(u32, gl::GL_COMPUTE_SHADER) }
        });
        g_compute[i].resizeLocalWorkGroup(0, 1, 1, 1);
        alive = alive && g_compute[i].compile();
    }
    ifcrashstr(!alive, "Unsuccessful shader compile");


    /* setup draw texture */
    for(auto& tex : g_texture) {
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_WRAP_S, gl::GL_CLAMP_TO_EDGE);
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_WRAP_T, gl::GL_CLAMP_TO_EDGE);
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_MIN_FILTER, gl::GL_NEAREST);
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_MAG_FILTER, gl::GL_NEAREST);
        gl::glTextureStorage2D(tex, 1, gl::GL_RGBA32F, g_dims.x, g_dims.y);
    }
    
    // initializeVectorFields(g_dims, g_initialField);
    // gl::glTextureSubImage2D(g_texture[0], 0, 0, 0, 
    //     g_dims.x, g_dims.y,
    //     gl::GL_RGBA, 
    //     gl::GL_FLOAT,
    //     g_initialField.data()
    // );


    gl::glNamedFramebufferTexture(g_fbo, gl::GL_COLOR_ATTACHMENT0, g_texture[2], 0);
    ifcrash( gl::glCheckNamedFramebufferStatus(g_fbo, gl::GL_FRAMEBUFFER) 
        != gl::GL_FRAMEBUFFER_COMPLETE 
    );


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
            render();
        }
        alive   = !awc2getContextStatus(contextid) && !awc2isKeyPressed(AWC2_KEYCODE_ESCAPE);
        paused ^= awc2isKeyPressed(AWC2_KEYCODE_P);
        awc2end();
    }
    markstr("Main App Loop End");


    /* return resources */
    markstr("Graphics Destroy Begin");
    gl::glDeleteTextures(__carraysize(g_texture), &g_texture[0]);
    gl::glDeleteFramebuffers(1, &g_fbo);
    for(auto& comp : g_compute) {
        comp.destroy();
    }
    g_initialField.resize(0); 
    markstr("Graphics Destroy End");

    markstr("AWC2 Destroy Begin");
    awc2destroyContext(contextid);
    awc2destroy();
    markstr("AWC2 Destroy End");



    markstr("compute_shader_render_buffer_to_screen end  ");
    return 1;
}