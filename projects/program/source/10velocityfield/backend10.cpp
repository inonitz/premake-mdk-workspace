#include "backend10.hpp"
#include <glbinding/gl/gl.h>
#include <imgui/imgui.h>
#include <awc2/C/awc2.h>
#include <util/marker2.hpp>
#include <threads.h>
#include <util/marker2.hpp>
#include <util/vec2.hpp>
#include <util/random.hpp>
#include "awc2/C/event.h"
#include "awc2/C/input.h"
#include "gl/shader2.hpp"



#if defined __linux__
    static constexpr const char* computeShaderFilename[8] = {
        "projects/program/source/10velocityfield/0externalforce.comp",
        "projects/program/source/10velocityfield/1advection.comp",
        "projects/program/source/10velocityfield/2diffusion.comp",
        "projects/program/source/10velocityfield/3addition.comp",
        "projects/program/source/10velocityfield/4divergence.comp",
        "projects/program/source/10velocityfield/5diffusion2.comp",
        "projects/program/source/10velocityfield/6velocity.comp",
        "projects/program/source/10velocityfield/7draw.comp"
    };
#elif defined _WIN32
    static constexpr const char* computeShaderFilename[8] = {
        "C:/CTools/Projects/main/projects/program/source/10velocityfield/0externalforce.comp",
        "C:/CTools/Projects/main/projects/program/source/10velocityfield/1advection.comp",
        "C:/CTools/Projects/main/projects/program/source/10velocityfield/2diffusion.comp",
        "C:/CTools/Projects/main/projects/program/source/10velocityfield/3addition.comp",
        "C:/CTools/Projects/main/projects/program/source/10velocityfield/4divergence.comp",
        "C:/CTools/Projects/main/projects/program/source/10velocityfield/5diffusion2.comp",
        "C:/CTools/Projects/main/projects/program/source/10velocityfield/6velocity.comp",
        "C:/CTools/Projects/main/projects/program/source/10velocityfield/7draw.comp"
    };
#endif
using namespace util::math;
static u8                 g_runCodeOnceFlag{true};
static u8                 g_contextid;
static Time::Timestamp    g_frameTime;
static std::vector<vec4f> g_initialField;
static vec2i              g_dims{1024, 1024};
static f32                g_dt        = 0.01f;
static f32                g_viscosity = 0.2f;
static i32                g_maximumJacobiIterations = 35;
static u32                g_texture[15];
static u32                g_fbo;
static ShaderProgramV2    g_compute[__carraysize(computeShaderFilename)];
static vec2f              g_mouseDragForce;
static vec2f              g_mouseDragPosition;
static f32                g_splatterRadius{0.1};
static vec4f              g_splatterColor{1.0f, 1.0f, 1.0f, 1.0f};
static bool               g_windowFocus{false};
static bool               g_mousePressed{false};
static bool               g_mouseDrawEvent{false};
static bool               g_slowRender{false};
static u32                g_slowRenderCounter{0};
static u32                g_frameCounter{0};


static ShaderProgramV2& gr_computeInteractive       = g_compute[0];
static ShaderProgramV2& gr_computeAdvection         = g_compute[1];
static ShaderProgramV2& gr_computeDiffusionVel      = g_compute[2];
static ShaderProgramV2& gr_computeAdd               = g_compute[3];
static ShaderProgramV2& gr_computeDivergence        = g_compute[4];
static ShaderProgramV2& gr_computeDiffusionPressure = g_compute[5];
static ShaderProgramV2& gr_computeNewVelocity       = g_compute[6];
static ShaderProgramV2& gr_computeRenderToTex       = g_compute[7];

static u32&             gr_drawTexture = g_texture[0];
static u32&             gr_dyeTexture0 = g_texture[1];
static u32&             gr_dyeTexture1 = g_texture[2];
static u32&             gr_tmpTexture0 = g_texture[3];
static u32&             gr_tmpTexture1 = g_texture[4];

static u32& gr_outTexShader0 = g_texture[5];
static u32& gr_outTexShader1 = g_texture[6];
static u32& gr_outTexShader2 = g_texture[7]; /* will use tmpTexture for ping-pong */
static u32& gr_outTexShader3 = g_texture[8];
static u32& gr_outTexShader4 = g_texture[9];
static u32& gr_outTexShader5 = g_texture[10];
static u32& gr_outTexShader6 = g_texture[11];
static u32& gr_simTexture0 = g_texture[12]; /* Components are [u.x, u.y, p, reserved] */
static u32& gr_simTexture1 = g_texture[13];




static void custom_mousebutton_callback(AWC2User_callback_mousebutton_struct const* data);
static void custom_winfocus_callback(AWC2User_callback_winfocus_struct const* data);
static void render_imgui_interface();
static u32  compute_fluid();
static void compute_velocity(u32 previousIteration, u32 nextIteration);
static void compute_dye(u32 previousIteration, u32 nextIteration);


u8               program::getContextID() { return g_contextid; }
Time::Timestamp& program::getFrameTime() { return g_frameTime; }
bool             program::getSlowRenderFlag() { return g_slowRender; }


void program::initializeLibrary()
{
    markstr("AWC2 init begin"); /* Init awc2 */
    awc2init();
    g_contextid = awc2createContext();
    AWC2ContextDescriptor ctxtinfo = {
        g_contextid,
        {0},
        __scast(u16, g_dims.x),
        __scast(u16, g_dims.y),
        AWC2WindowDescriptor{}
    };
    awc2initializeContext(&ctxtinfo);
    awc2setContextUserCallbackMouseButton(g_contextid, &custom_mousebutton_callback);
    awc2setContextUserCallbackWindowFocus(g_contextid, &custom_winfocus_callback);
    markstr("AWC2 init end");
    return;
}


void program::destroyLibrary()
{
    markstr("AWC2 Destroy Begin");
    awc2destroyContext(g_contextid);
    awc2destroy();
    markstr("AWC2 Destroy End");
    return;
}




void program::initializeGraphics()
{
    u8 alive{true};


    markstr("Graphics Init Begin");
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


    for(auto& tex : g_texture) {
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_WRAP_S, gl::GL_CLAMP_TO_EDGE);
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_WRAP_T, gl::GL_CLAMP_TO_EDGE);
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_MIN_FILTER, gl::GL_NEAREST);
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_MAG_FILTER, gl::GL_NEAREST);
        gl::glTextureStorage2D(tex, 1, gl::GL_RGBA32F, g_dims.x, g_dims.y);
        gl::glClearTexImage(tex, 0, gl::GL_RGBA, gl::GL_FLOAT, nullptr);
    }


    gl::glNamedFramebufferTexture(g_fbo, gl::GL_COLOR_ATTACHMENT0, gr_drawTexture, 0);
    ifcrash( gl::glCheckNamedFramebufferStatus(g_fbo, gl::GL_FRAMEBUFFER) 
        != gl::GL_FRAMEBUFFER_COMPLETE 
    );


    markstr("Graphics Init End  ");
    return;
}


void program::destroyGraphics()
{
    markstr("Graphics Destroy Begin");


    gl::glDeleteTextures(__carraysize(g_texture), &g_texture[0]);
    gl::glDeleteFramebuffers(1, &g_fbo);
    for(auto& comp : g_compute) {
        comp.destroy();
    }
    g_initialField.resize(0); 
    
    
    markstr("Graphics Destroy End");
    return;
}


void program::render()
{
    static const vec4f defaultScreenColor = vec4f{1.0f, 1.0f, 1.0f, 1.0f}; 
    auto  currentWindowSize = awc2getCurrentContextViewport();
    u8    status = 1;


    /* shader refresh needs a rework to be more relevant, literally no point updating 9 shaders every time */
    if(awc2isKeyPressed(AWC2_KEYCODE_R)) {
        markstr("Refreshing Compute Shaders...");
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
    u32 texToRender = compute_fluid();


    /* literally just the velocity texture on the screen */
    gr_computeRenderToTex.bind();
    gr_computeRenderToTex.uniform1i("fieldSampler", 0);
    gr_computeRenderToTex.uniform1i("screentexture", 1);
    gr_computeRenderToTex.uniform2iv("ku_simdims", g_dims.begin());
    gl::glBindTextureUnit(0, texToRender);
    gl::glBindImageTexture(1, gr_drawTexture, 0, false, 0, 
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
    


    g_mousePressed    = false;
    ++g_frameCounter;
    return;
}


static void render_imgui_interface()
{
    if(awc2isMouseButtonPressed(AWC2_MOUSEBUTTON_LEFT)) {
        AWC2CursorPosition delta = awc2getMousePositionDelta();
        AWC2CursorPosition pos = awc2getMousePosition();

        g_mouseDragForce    = vec2f{ delta.x, delta.y };
        g_mouseDragPosition = vec2f{ pos.x, pos.y };
        g_mousePressed      = true;
    }
    if(awc2isMouseButtonPressed(AWC2_MOUSEBUTTON_RIGHT)) {
        if(g_runCodeOnceFlag) {
            g_splatterColor = vec4f{ random32f(), random32f(), 0.0f, 1.0f };
            g_runCodeOnceFlag = false;
        }
    }
    g_runCodeOnceFlag = awc2isMouseButtonReleased(AWC2_MOUSEBUTTON_RIGHT);


    char* dragForceStr    = g_mouseDragForce.to_string();
    char* mouseDragPosStr = g_mouseDragPosition.to_string();

    ImGui::Begin("Simulation Parameters");
    ImGui::Text("\
Mouse Dragging Force %s\n\
Mouse Position       %s\n\
Mouse was Pressed    %u\n\
FrameCounter         %u (%2.4f ms)\n\
",
    g_mouseDragForce.to_string(), 
    mouseDragPosStr,
    g_mousePressed,
    g_frameCounter,
    g_frameTime.value_units<f32>(1000)
    );
    if(ImGui::Button("Slow Render")) {
        g_slowRender = boolean((g_slowRenderCounter % 2) == 0);
        ++g_slowRenderCounter;
    }
    ImGui::DragFloat("Delta Time",     &g_dt,             0.01f, 0.001f, 0.2f);
    ImGui::DragFloat("Viscosity",      &g_viscosity,      0.01f, 0.01f,  4.0f);
    ImGui::DragFloat("splatterRadius", &g_splatterRadius, 0.01f, 0.01f,  0.5f);
    ImGui::DragInt("Diffusion Solver Iterations", &g_maximumJacobiIterations, 0.5f, 10, 80);
    ImVec4* colorcvt = __rcast(ImVec4*, g_splatterColor.begin());
    ImGui::ColorButton("splatterColor ", *colorcvt);
    ImGui::End();

    free(dragForceStr);
    free(mouseDragPosStr);
    return;
}


static u32 compute_fluid()
{
    u32 previousIteration, nextIteration;
    if(g_frameCounter % 2) {
        previousIteration = gr_simTexture1;
        nextIteration     = gr_simTexture0;
    } else {
        previousIteration = gr_simTexture0;
        nextIteration     = gr_simTexture1;
    }


    compute_velocity(previousIteration, nextIteration);
    compute_dye(0, 0);
    return nextIteration;
}


static void compute_velocity(u32 previousIteration, u32 nextIteration)
{
    /* write user-mouse interaction to force texture (gr_outTexShader0), not adding dye sources just yet */
    //  if(g_mousePressed && awc2isMouseButtonPressed(AWC2_MOUSEBUTTON_RIGHT)) {
    if(g_mousePressed && awc2isMouseButtonPressed(AWC2_MOUSEBUTTON_RIGHT)) {
        gr_computeInteractive.bind();
        gr_computeInteractive.uniform1i("pong", 0);
        gr_computeInteractive.uniform1f("ku_dt",       g_dt);
        gr_computeInteractive.uniform2iv("ku_simdims", g_dims.begin());
        gr_computeInteractive.uniform2fv("ku_mouseDragForce", g_mouseDragForce.begin()   );
        gr_computeInteractive.uniform2fv("ku_mouseDragPos",   g_mouseDragPosition.begin());
        gr_computeInteractive.uniform1f("ku_splatterRadius",  g_splatterRadius); 
        gr_computeInteractive.uniform4fv("ku_splatterColor",  g_splatterColor.begin());
        gr_computeInteractive.uniform1ui("ku_mousePressed",   g_mousePressed);
        gl::glBindImageTexture(0, gr_outTexShader0, 0, false, 0, 
            gl::GL_WRITE_ONLY, 
            gl::GL_RGBA32F
        );
        gl::glDispatchCompute(g_dims.x, g_dims.y, 1);
    }


    /* Compute diffuse component of the velocity field to diffusion texture (gr_outTexShader2) */
    u32 textureInput, textureOutput;
    gr_computeDiffusionVel.bind();
    gr_computeDiffusionVel.uniform1i("initialField", 0);
    gr_computeDiffusionVel.uniform1i("outputField",  1);
    gr_computeDiffusionVel.uniform1f("ku_dt",        g_dt);
    gr_computeDiffusionVel.uniform1f("ku_viscosity", g_viscosity);
    gr_computeDiffusionVel.uniform2iv("ku_simdims", g_dims.begin());
    gl::glBindTextureUnit(0, previousIteration);
    gl::glBindImageTexture(1, gr_outTexShader2, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    gl::glDispatchCompute(g_dims.x, g_dims.y, 1);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);


    textureInput  = gr_outTexShader2;
    textureOutput = gr_tmpTexture0;
    for(i32 i = 1; i < g_maximumJacobiIterations; ++i) {
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


    /* add all the shit we computed together for the next iteration */
    gr_computeAdd.bind();
    gr_computeAdd.uniform1i("externuserField", 0);
    gr_computeAdd.uniform1i("advectedField",   1);
    gr_computeAdd.uniform1i("diffusedField",   2);
    gr_computeAdd.uniform1i("outputField",     3);
    gl::glBindTextureUnit(0, gr_outTexShader0);
    gl::glBindTextureUnit(1, gr_outTexShader1);
    gl::glBindTextureUnit(2, gr_outTexShader2);
    gl::glBindImageTexture(3, gr_outTexShader3, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    gl::glDispatchCompute(g_dims.x, g_dims.y, 1);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);


    /* 
        Compute the divergence of the previous iteration. 
        We write the result to the w component of the texture (1-1 copy but .w is div(gr_outTexShader3) ) 
        resulting field in gr_outTexShader4
    */
    gr_computeDivergence.bind();
    gr_computeDivergence.uniform1i("intermediateVelocityField", 0);
    gr_computeDivergence.uniform1i("oldPressureFieldValue",     1);
    gr_computeDivergence.uniform1i("outputField",               2);
    gl::glBindTextureUnit(0, gr_outTexShader3);
    gl::glBindTextureUnit(1, previousIteration);
    gl::glBindImageTexture(2, gr_outTexShader4, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    gl::glDispatchCompute(g_dims.x, g_dims.y, 1);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);



    /* Compute diffuse component of the pressure field to .z component (gr_outTexShader5) */
    gr_computeDiffusionPressure.bind();
    gr_computeDiffusionPressure.uniform1i("oldPressureField", 0);
    gr_computeDiffusionPressure.uniform1i("newPressureField", 1);
    gr_computeDiffusionPressure.uniform2iv("ku_simdims", g_dims.begin());
    gl::glBindTextureUnit(0, gr_outTexShader4);
    gl::glBindImageTexture(1, gr_outTexShader5, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    gl::glDispatchCompute(g_dims.x, g_dims.y, 1);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);


    textureInput  = gr_outTexShader5;
    textureOutput = gr_tmpTexture0;
    for(i32 i = 1; i < g_maximumJacobiIterations; ++i) {
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
    // gl::glBindTextureUnit(0, textureInput);
    // gl::glBindImageTexture(1, nextIteration, 0, false, 0, 
    //     gl::GL_WRITE_ONLY, 
    //     gl::GL_RGBA32F
    // );
    // gl::glDispatchCompute(g_dims.x, g_dims.y, 1);
    // gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);


    /* add together all the shit we computed for the next iteration */
    gr_computeNewVelocity.bind();
    gr_computeNewVelocity.uniform1i("intermediateWPdivW",          0);
    gr_computeNewVelocity.uniform1i("updatedVelocityWithPressure", 1);
    gl::glBindTextureUnit(0, gr_outTexShader5);
    gl::glBindImageTexture(1, nextIteration, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    gl::glDispatchCompute(g_dims.x, g_dims.y, 1);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
    return;
}


static void compute_dye(u32 previousIteration, u32 nextIteration)
{
    return;
}



static void custom_mousebutton_callback(AWC2User_callback_mousebutton_struct const* data)
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


static void custom_winfocus_callback(AWC2User_callback_winfocus_struct const* data) 
{
    g_windowFocus = data->focused;
    return;
}