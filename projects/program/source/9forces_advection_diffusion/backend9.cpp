#include "backend9.hpp"
#include <glbinding/gl/gl.h>
#include <imgui/imgui.h>
#include <awc2/C/awc2.h>
#include <util2/C/marker4.h>
#include <util2/vec2.hpp>
#include "gl/shader2.hpp"
#include <util2/C/marker4.h>


#if defined __linux__
    static constexpr const char* computeShaderFilename[5] = {
        "projects/program/source/9forces_advection_diffusion/0externalforce.comp",
        "projects/program/source/9forces_advection_diffusion/1advection.comp",
        "projects/program/source/9forces_advection_diffusion/2diffusion.comp",
        "projects/program/source/9forces_advection_diffusion/3addition.comp",
        "projects/program/source/9forces_advection_diffusion/4draw.comp"
    };
#elif defined _WIN32
    static constexpr const char* computeShaderFilename[5] = {
        "C:/CTools/Projects/main/projects/program/source/9forces_advection_diffusion/0externalforce.comp",
        "C:/CTools/Projects/main/projects/program/source/9forces_advection_diffusion/1advection.comp",
        "C:/CTools/Projects/main/projects/program/source/9forces_advection_diffusion/2diffusion.comp",
        "C:/CTools/Projects/main/projects/program/source/9forces_advection_diffusion/3addition.comp",
        "C:/CTools/Projects/main/projects/program/source/9forces_advection_diffusion/4draw.comp"
    };
#endif
using namespace util2::math;
static u8                 g_contextid;
static util2::Time::Timestamp    g_frameTime;   
static std::vector<vec4f> g_initialField;
static vec2i              g_dims{1024, 1024};
static f32                g_dt        = 0.01f;
static f32                g_viscosity = 0.2f;
static u32                g_maximumJacobiIterations = 40;
static u32                g_texture[10];
static u32                g_fbo;
static ShaderProgramV2    g_compute[__carraysize(computeShaderFilename)];
static vec2f              g_mouseDragForce;
static vec2f              g_mouseDragPosition;
static f32                g_splatterRadius{0.1};
static vec4f              g_splatterColor{1.0f, 1.0f, 1.0f, 1.0f};
static bool               g_mousePressed{false};
static u32                g_frameCounter{0};


static ShaderProgramV2& gr_computeInteractive = g_compute[0];
static ShaderProgramV2& gr_computeAdvection   = g_compute[1];
static ShaderProgramV2& gr_computeDiffusion   = g_compute[2];
static ShaderProgramV2& gr_computeAdd         = g_compute[3];
static ShaderProgramV2& gr_computeRenderToTex = g_compute[4];

static u32&             gr_drawTexture = g_texture[0];
static u32&             gr_dyeTexture0 = g_texture[1];
static u32&             gr_dyeTexture1 = g_texture[2];
static u32&             gr_tmpTexture0 = g_texture[3];
static u32&             gr_tmpTexture1 = g_texture[4];

static u32& gr_outTexShader0 = g_texture[5];
static u32& gr_outTexShader1 = g_texture[6];
static u32& gr_outTexShader2 = g_texture[7]; /* will use tmpTexture for ping-pong */
static u32& gr_outTexShader3 = g_texture[8];




static void custom_mousebutton_callback(AWC2User_callback_mousebutton_struct const* data);
static void render_imgui_interface();
static void compute_fluid();
static void compute_velocity();
static void compute_dye();


u8               program0::getContextID() { return g_contextid; }
util2::Time::Timestamp& program0::getFrameTime() { return g_frameTime; }


void program0::initializeLibrary()
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
    markstr("AWC2 init end");
    return;
}


void program0::destroyLibrary()
{
    markstr("AWC2 Destroy Begin");
    awc2destroyContext(g_contextid);
    awc2destroy();
    markstr("AWC2 Destroy End");
    return;
}




void program0::initializeGraphics()
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
    }
    gl::glClearTexImage(gr_dyeTexture0, 0, gl::GL_RGBA, gl::GL_FLOAT, nullptr);
    gl::glClearTexImage(gr_dyeTexture1, 0, gl::GL_RGBA, gl::GL_FLOAT, nullptr);
    gl::glClearTexImage(gr_outTexShader0, 0, gl::GL_RGBA, gl::GL_FLOAT, nullptr);
    gl::glClearTexImage(gr_outTexShader1, 0, gl::GL_RGBA, gl::GL_FLOAT, nullptr);
    gl::glClearTexImage(gr_outTexShader2, 0, gl::GL_RGBA, gl::GL_FLOAT, nullptr);


    gl::glNamedFramebufferTexture(g_fbo, gl::GL_COLOR_ATTACHMENT0, gr_drawTexture, 0);
    ifcrash( gl::glCheckNamedFramebufferStatus(g_fbo, gl::GL_FRAMEBUFFER) 
        != gl::GL_FRAMEBUFFER_COMPLETE 
    );


    markstr("Graphics Init End  ");
    return;
}


void program0::destroyGraphics()
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


void program0::render()
{
    static const vec4f defaultScreenColor = vec4f{1.0f, 1.0f, 1.0f, 1.0f}; 
    auto  currentWindowSize = awc2getCurrentContextViewport();
    u8    status = 1;


    /* shader refresh needs a rework to be more relevant, literally no point updating 9 shaders every time */
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
    compute_fluid();


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


static void render_imgui_interface()
{
    if(awc2isMouseButtonPressed(AWC2_MOUSEBUTTON_LEFT)) {
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
Mouse was Pressed    %u\n\
FrameCounter         %u (%2.4f ms)\n\
",
    g_mouseDragForce.to_string(), 
    mouseDragPosStr,
    g_mousePressed,
    g_frameCounter,
    g_frameTime.value_units<f32>(1000)
    );
    ImGui::DragFloat("splatterRadius",        &g_splatterRadius, 0.01f, 0.01f, 0.5f);
    ImGui::ColorEdit4("splatterColor",        g_splatterColor.begin());
    ImGui::DragFloat("Delta Time",            &g_dt,             0.01f, 0.001, 0.2 );
    ImGui::DragFloat("Viscosity Coefficient", &g_viscosity,      0.01f, 0.01f, 1.0f);
    ImGui::End();


    free(dragForceStr);
    free(mouseDragPosStr);
    return;
}


static void compute_fluid()
{
    u32 previousIteration, nextIteration;
    u32 textureInput, textureOutput;
    if(!(g_frameCounter % 2)) {
        previousIteration = gr_dyeTexture0;
        nextIteration     = gr_dyeTexture1;
    } else {
        previousIteration = gr_dyeTexture1;
        nextIteration     = gr_dyeTexture0;
    }
    // gr_tmpTexture0 = g_texture[3];
    // gr_tmpTexture1 = g_texture[4];




    /* write dye sources to some texture */
    if(awc2isMouseButtonPressed(AWC2_MOUSEBUTTON_LEFT) && awc2isMouseButtonPressed(AWC2_MOUSEBUTTON_RIGHT)) {
        gr_computeInteractive.bind();
        gr_computeInteractive.uniform1i("pong", 0);
        gr_computeInteractive.uniform1f("ku_dt",        g_dt);
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

    /* apply advection to previousIteration, write to some other texture */
    // gr_computeAdvection.bind();
    // gr_computeAdvection.uniform1i("initialField", 0);
    // gr_computeAdvection.uniform1i("outputField",  1);
    // gr_computeAdvection.uniform1f("ku_dt",        g_dt);
    // gr_computeAdvection.uniform2iv("ku_simdims", g_dims.begin());
    // gl::glBindTextureUnit(0, previousIteration);
    // gl::glBindImageTexture(1, gr_outTexShader1, 0, false, 0, 
    //     gl::GL_WRITE_ONLY, 
    //     gl::GL_RGBA32F
    // );
    // gl::glDispatchCompute(g_dims.x, g_dims.y, 1);


    // /* apply diffusion to whatever's inside previousIteration, write it to some other texture */
    gr_computeDiffusion.bind();
    gr_computeDiffusion.uniform1i("initialField", 0);
    gr_computeDiffusion.uniform1i("outputField",  1);
    gr_computeDiffusion.uniform1f("ku_dt",        g_dt);
    gr_computeDiffusion.uniform1f("ku_viscosity", g_viscosity);
    gr_computeDiffusion.uniform2iv("ku_simdims", g_dims.begin());
    gl::glBindTextureUnit(0, previousIteration);
    gl::glBindImageTexture(1, gr_outTexShader2, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    gl::glDispatchCompute(g_dims.x, g_dims.y, 1);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);


    textureInput  = gr_outTexShader2;
    textureOutput = gr_tmpTexture0;
    for(u32 i = 1; i < g_maximumJacobiIterations; ++i) {
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
    gl::glBindImageTexture(3, nextIteration, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    gl::glDispatchCompute(g_dims.x, g_dims.y, 1);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);


    /* literally just show that other texture on the screen */
    gr_computeRenderToTex.bind();
    gr_computeRenderToTex.uniform1i("fieldSampler", 0);
    gr_computeRenderToTex.uniform1i("screentexture", 1);
    gr_computeRenderToTex.uniform2iv("ku_simdims", g_dims.begin());
    gl::glBindTextureUnit(0, nextIteration);
    gl::glBindImageTexture(1, gr_drawTexture, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    gl::glDispatchCompute(g_dims.x, g_dims.y, 1);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
}


static void compute_velocity()
{

}
static void compute_dye()
{

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