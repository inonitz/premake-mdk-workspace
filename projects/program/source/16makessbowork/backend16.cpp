#include "backend16.hpp"
#include <glbinding/gl46core/gl.h>
#include <imgui/imgui.h>
#include <awc2/C/awc2.h>
#include <util/marker2.hpp>
#include <threads.h>
#include <util/vec2.hpp>
#include <util/marker2.hpp>
#include <util/random.hpp>
#include <util/time.hpp>
#include "gl/shader2.hpp"



static constexpr const char* computeShaderFilename[9] = {
    "projects/program/source/16makessbowork/0externalforce.comp",
    "projects/program/source/16makessbowork/1advection.comp",
    "projects/program/source/16makessbowork/2double_diffusion.comp",
    "projects/program/source/16makessbowork/3addition.comp",
    "projects/program/source/16makessbowork/4divergence.comp",
    "projects/program/source/16makessbowork/5double_diffusion2.comp",
    "projects/program/source/16makessbowork/6velocity.comp",
    "projects/program/source/16makessbowork/7draw.comp",
    "projects/program/source/16makessbowork/parallelReductionSSBO.comp"
};
using namespace util::math;
static u8                 g_runCodeOnceFlag{true};
static u8                 g_contextid;
static Time::Timestamp    g_frameTime{};
static Time::Timestamp    g_measuremisc0;
static Time::Timestamp    g_measuremisc1;
static Time::Timestamp    g_renderTime{};
static Time::Timestamp    g_computeVelTime{};
static Time::Timestamp    g_computeCFLTime{};
static Time::Timestamp    g_computeFluidTime{};
static Time::Timestamp    g_retrieveTextureData{};
static Time::Timestamp    g_computeMaximum{};
static Time::Timestamp    g_renderImguiTime;
static Time::Timestamp    g_renderScreenTime;
static Time::Timestamp    g_refreshShaderTime;


static std::vector<vec4f> g_initialField;
static vec2i              g_dims{1024, 1024};
static vec2i              g_windims{g_dims};
static vec2f              g_simUnitCoords{1.0f};
static f32                g_dt        = 0.2f;
static f32                g_viscosity = 1.0f;
static i32                g_maximumJacobiIterations = 35;
static i32                g_reductionFactor = 4096;
static i32                g_reductionBufferLength = g_dims.x * g_dims.y / g_reductionFactor;
static std::vector<vec4f> g_reductionBuffer;
static vec2f              g_velocityCFL{0.0f, 0.0f};

static gl::GLsync         g_fence; 
static u32                g_texture[19];
static u32                g_persistentbuf[2];
static void*              g_mappedBuffer[2];
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
static ShaderProgramV2& gr_computeCFLCondition      = g_compute[8];


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
static u32& gr_outTexShader7 = g_texture[12];
static u32& gr_outTexShader8 = g_texture[13];
static u32& gr_outTexShader9 = g_texture[14];

static u32& gr_simTexture0 = g_texture[15]; /* Components are [u.x, u.y, p, reserved] */
static u32& gr_simTexture1 = g_texture[16];
static u32& g_reductionMinTexture = g_persistentbuf[0];
static u32& g_reductionMaxTexture = g_persistentbuf[1];
static void* g_reductionMaxMappedBuf = nullptr;



static void custom_mousebutton_callback(AWC2User_callback_mousebutton_struct const* data);
static void custom_winfocus_callback(AWC2User_callback_winfocus_struct const* data);
static void render_imgui_interface();
static u32  compute_fluid();
static void compute_velocity(
    u32 previousIteration, 
    u32 nextIteration
);
static void compute_dye(
    u32 velocityPressureField, 
    u32 previousIteration,
    u32 nextIteration
);
static void compute_cfl(u32 texture);


u8               ssbowork::getContextID()  { return g_contextid; }
Time::Timestamp& ssbowork::getFrameTime()  { return g_frameTime; }
Time::Timestamp& ssbowork::getRenderTime() { return g_renderTime; }
Time::Timestamp& ssbowork::getTimer0() { return g_measuremisc0; }
Time::Timestamp& ssbowork::getTimer1() { return g_measuremisc1; }
bool             ssbowork::getSlowRenderFlag() { return g_slowRender; }


void ssbowork::initializeLibrary()
{
    markstr("AWC2 init begin"); /* Init awc2 */
    awc2init();
    g_contextid = awc2createContext();
    auto desc = AWC2WindowDescriptor{};
    // desc.createFlags |= AWC2_WINDOW_CREATION_FLAG_USE_VSYNC;
    // desc.refreshRate = 280;
    AWC2ContextDescriptor ctxtinfo = {
        g_contextid,
        {0},
        __scast(u16, g_dims.x),
        __scast(u16, g_dims.y),
        desc
    };
    awc2initializeContext(&ctxtinfo);
    awc2setContextUserCallbackMouseButton(g_contextid, &custom_mousebutton_callback);
    awc2setContextUserCallbackWindowFocus(g_contextid, &custom_winfocus_callback);
    markstr("AWC2 init end");
    return;
}


void ssbowork::destroyLibrary()
{
    markstr("AWC2 Destroy Begin");
    awc2destroyContext(g_contextid);
    awc2destroy();
    markstr("AWC2 Destroy End");
    return;
}




void ssbowork::initializeGraphics()
{
    u8 alive{true};


    markstr("Graphics Init Begin");
    gl::glCreateTextures(gl::GL_TEXTURE_2D, __carraysize(g_texture), &g_texture[0]);
    gl::glCreateBuffers(2, &g_persistentbuf[0]);
    gl::glCreateFramebuffers(1, &g_fbo);


    for(uint32_t i = 0; alive && i < __carraysize(computeShaderFilename); ++i) {
        g_compute[i].createFrom({
            ShaderData{ computeShaderFilename[i], __scast(u32, gl::GL_COMPUTE_SHADER) }
        });
        g_compute[i].resizeLocalWorkGroup(0, 1, 1, 1);
        alive = alive && g_compute[i].compile();
    }
    ifcrashstr(!alive, "Unsuccessful shader compile");


    for(u32 i = 0; i < __carraysize(g_texture); ++i) {
        auto& tex = g_texture[i];
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_WRAP_S, gl::GL_CLAMP_TO_EDGE);
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_WRAP_T, gl::GL_CLAMP_TO_EDGE);
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_MIN_FILTER, gl::GL_NEAREST);
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_MAG_FILTER, gl::GL_NEAREST);
        gl::glTextureStorage2D(tex, 1, gl::GL_RGBA32F, g_dims.x, g_dims.y);
        gl::glClearTexImage(tex, 0, gl::GL_RGBA, gl::GL_FLOAT, nullptr);
    }


    gl::glNamedBufferStorage(g_reductionMaxTexture, 
        g_reductionBufferLength * sizeof(vec4f), 
        nullptr, 
        gl::GL_NONE_BIT
        | gl::GL_MAP_READ_BIT
        | gl::GL_MAP_PERSISTENT_BIT
        | gl::GL_MAP_COHERENT_BIT
    );
    g_reductionMaxMappedBuf = gl::glMapNamedBufferRange(g_reductionMaxTexture, 
        0, g_reductionBufferLength * sizeof(vec4f),  
        gl::GL_NONE_BIT
        | gl::GL_MAP_READ_BIT
        | gl::GL_MAP_PERSISTENT_BIT
        | gl::GL_MAP_COHERENT_BIT
    );


    gl::glNamedFramebufferTexture(g_fbo, gl::GL_COLOR_ATTACHMENT0, gr_drawTexture, 0);
    ifcrash( gl::glCheckNamedFramebufferStatus(g_fbo, gl::GL_FRAMEBUFFER) 
        != gl::GL_FRAMEBUFFER_COMPLETE 
    );


    markstr("Graphics Init End  ");
    return;
}


void ssbowork::destroyGraphics()
{
    markstr("Graphics Destroy Begin");


    gl::glDeleteTextures(__carraysize(g_texture), &g_texture[0]);
    gl::glUnmapNamedBuffer(g_reductionMaxTexture);
    gl::glDeleteBuffers(__carraysize(g_persistentbuf), &g_persistentbuf[0]);
    gl::glDeleteFramebuffers(1, &g_fbo);
    for(auto& comp : g_compute) {
        comp.destroy();
    }
    g_initialField.resize(0); 
    g_reductionBuffer.resize(0);
    

    markstr("Graphics Destroy End");
    return;
}


void ssbowork::render()
{
    static const vec4f defaultScreenColor = vec4f{1.0f, 1.0f, 1.0f, 1.0f}; 
    auto  currentWindowSize = awc2getCurrentContextViewport();
    u8    status = 1;


    g_refreshShaderTime.begin();
    g_windims = vec2i{ currentWindowSize.x, currentWindowSize.y };
    /* shader refresh needs a rework to be more relevant, literally no point updating 9 shaders every time */
    if(awc2isKeyPressed(AWC2_KEYCODE_R)) {
        markstr("Refreshing Compute Shaders...");
        for(auto& comp : g_compute) {
            comp.refreshFromFiles();
            comp.resizeLocalWorkGroup(0, 1, 1, 1);
            status = status && comp.compile();
        }
    }
    if(!status)
        return; /* couldn't recompile successfully, code needs recheck */

    g_refreshShaderTime.end();


    gl::glClearNamedFramebufferfv( /* Clear Screen */
        g_fbo, 
        gl::GL_COLOR, 
        0, 
        defaultScreenColor.begin()
    );


    u32 texToRender = 0;
    TIME_NAMESPACE_TIME_CODE_BLOCK(g_renderImguiTime, render_imgui_interface());
    TIME_NAMESPACE_TIME_CODE_BLOCK(g_computeFluidTime, texToRender = compute_fluid());


    /* literally just the velocity texture on the screen */
    g_renderScreenTime.begin();
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
    g_renderScreenTime.end();


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
    if(awc2isMouseButtonPressed(AWC2_MOUSEBUTTON_RIGHT)) {
        if(g_runCodeOnceFlag) {
            g_splatterColor = vec4f{ random32f(), random32f(), 0.0f, 1.0f };
            g_runCodeOnceFlag = false;
        }
    }
    g_runCodeOnceFlag = awc2isMouseButtonReleased(AWC2_MOUSEBUTTON_RIGHT);

    vec2f cfl = g_velocityCFL / g_simUnitCoords;
    char* dragForceStr    = g_mouseDragForce.to_string();
    char* mouseDragPosStr = g_mouseDragPosition.to_string();
    char* simDimsStr      = g_dims.to_string();
    char* winDimsStr      = g_windims.to_string();
    char* maxVelStr       = g_velocityCFL.to_string();
    static f32 timeMeasurements[16] = {0};


    timeMeasurements[0] = g_frameTime.value_units<f32>(1000);
    timeMeasurements[1] = g_renderTime.value_units<f32>(1000);
    timeMeasurements[2] = g_computeFluidTime.value_units<f32>(1000);
    timeMeasurements[3] = g_computeVelTime.value_units<f32>(1000);
    timeMeasurements[4] = g_computeCFLTime.value_units<f32>(1000);
    timeMeasurements[5] = g_retrieveTextureData.value_units<f32>(1000);
    timeMeasurements[6] = g_computeMaximum.value_units<f32>(1000);
    timeMeasurements[7] = timeMeasurements[4] - timeMeasurements[5] - timeMeasurements[6];
    timeMeasurements[8] = timeMeasurements[2] - timeMeasurements[3] - timeMeasurements[4];
    timeMeasurements[9] = g_refreshShaderTime.value_units<f32>(1000);
    timeMeasurements[10] = g_renderImguiTime.value_units<f32>(1000);
    timeMeasurements[11] = g_renderScreenTime.value_units<f32>(1000);
    timeMeasurements[12] = g_measuremisc0.value_units<f32>(1000);
    timeMeasurements[13] = g_measuremisc1.value_units<f32>(1000);


    ImGui::Begin("Simulation Parameters");
    ImGui::Text("\
Mouse Dragging Force %s\n\
Mouse Position       %s\n\
Mouse was Pressed    %u\n\
FrameCounter         %u (%6.4f ms)\n\
Compute Time\n\
    [awc2end] { \n\
        %6.4f, \n\
        %6.4f, \n\
        %6.4f, \n\
        %6.4f, \n\
        %6.4f \n\
    }\n\
    %6.4f [misc0]\n\
    %6.4f [misc1]\n\
    %6.4f [render]\n\
    - %6.4f [fluid]\n\
        - %6.4f [velocity]\n\
        - %6.4f [cfl]\n\
            - %6.4f [retrieve]\n\
            - %6.4f [for_loop]\n\
            - %6.4f [remainder]\n\
        - %6.4f [remainder]\n\
    - %6.4f [shader_refresh]\n\
    - %6.4f [render_imgui]\n\
    - %6.4f [compute_screen+blit]\n\
Simulation Dims      %s\n\
Window     Dims      %s\n\
Maximum Velocity     %s\n\
CFL Condition (< 1)  %9.6f\n\
",
    dragForceStr, 
    mouseDragPosStr,
    g_mousePressed,
    g_frameCounter,
    timeMeasurements[0],
    Time::getGeneralPurposeStamp(0).value_units<f32>(1000),
    Time::getGeneralPurposeStamp(1).value_units<f32>(1000),
    Time::getGeneralPurposeStamp(2).value_units<f32>(1000),
    Time::getGeneralPurposeStamp(3).value_units<f32>(1000),
    Time::getGeneralPurposeStamp(4).value_units<f32>(1000),
    timeMeasurements[12],
    timeMeasurements[13],
    timeMeasurements[1],
    timeMeasurements[2],
    timeMeasurements[3],
    timeMeasurements[4],
    timeMeasurements[5],
    timeMeasurements[6],
    timeMeasurements[7],
    timeMeasurements[8],
    timeMeasurements[9],
    timeMeasurements[10],
    timeMeasurements[11],
    simDimsStr,
    winDimsStr,
    maxVelStr,
    (cfl.x + cfl.y) * g_dt
);
    if(ImGui::Button("Slow Render")) {
        g_slowRender = boolean((g_slowRenderCounter % 2) == 0);
        ++g_slowRenderCounter;
    }
    if(ImGui::Button("Restart Simulation")) {
        for(auto& tex : g_texture) {
            gl::glClearTexImage(tex, 0, gl::GL_RGBA, gl::GL_FLOAT, nullptr);
        }
        g_velocityCFL = vec2f{0.0f};
    }
    ImGui::DragFloat2("Unit Coordinate Size", g_simUnitCoords.begin(), 0.1f, 0.1f, 100.0f);
    ImGui::DragFloat("Delta Time",     &g_dt,             0.001f, 0.001f,  0.5f, "%.6f");
    ImGui::DragFloat("Viscosity",      &g_viscosity,      0.01f,  0.1f,   10.0f, "%.6f");
    ImGui::DragFloat("splatterRadius", &g_splatterRadius, 0.001f, 0.001f, 0.2f,  "%.6f");
    ImGui::DragInt("Diffusion Solver Iterations", &g_maximumJacobiIterations, 0.5f, 10, 80);
    ImVec4* colorcvt = __rcast(ImVec4*, g_splatterColor.begin());
    ImGui::ColorButton("splatterColor ", *colorcvt);
    ImGui::End();

    free(mouseDragPosStr);
    free(dragForceStr);
    free(simDimsStr);
    free(winDimsStr);
    free(maxVelStr);

    /* 
        g_velocityCFL is from the previous frame. 
        if I reset it after calculating it at compute_cfl() 
        then render_imgui_interface() won't have a chance to render it =>
        after I render the value, I zero it because it's only used for debug purposes now.
    */
    g_velocityCFL = vec2f{0.0f};
    return;
}


static u32 compute_fluid()
{
    u32 previousIterationVel, nextIterationVel;
    u32 previousIterationDye, nextIterationDye;
    if(g_frameCounter % 2) {
        previousIterationVel = gr_simTexture1;
        nextIterationVel     = gr_simTexture0;

        previousIterationDye = gr_dyeTexture1;
        nextIterationDye     = gr_dyeTexture0;
    } else {
        previousIterationVel = gr_simTexture0;
        nextIterationVel     = gr_simTexture1;

        previousIterationDye = gr_dyeTexture0;
        nextIterationDye     = gr_dyeTexture1;
    }


    TIME_NAMESPACE_TIME_CODE_BLOCK(g_computeVelTime, compute_velocity(previousIterationVel, nextIterationVel));
    TIME_NAMESPACE_TIME_CODE_BLOCK(g_computeCFLTime, compute_cfl(nextIterationVel));
    TIME_NAMESPACE_TIME_CODE_BLOCK(Time::getGeneralPurposeStamp(4), (void()));

    return nextIterationVel;
}


static void compute_velocity(u32 previousIteration, u32 nextIteration)
{
    bool userInteractionFlag = g_mousePressed && awc2isMouseButtonPressed(AWC2_MOUSEBUTTON_RIGHT);
    if(likely(!userInteractionFlag)) {
        gl::glClearTexImage(gr_outTexShader0, 0, gl::GL_RGBA, gl::GL_FLOAT, nullptr);
    } else {
        gr_computeInteractive.bind();
        gr_computeInteractive.uniform1i("pong", 0);
        gr_computeInteractive.uniform1f("ku_dt",       g_dt);
        gr_computeInteractive.uniform2iv("ku_simdims", g_dims.begin());
        gr_computeInteractive.uniform2iv("ku_windims", g_windims.begin());
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


    gr_computeAdvection.bind();
    gr_computeAdvection.uniform1i("quantityField", 0);
    gr_computeAdvection.uniform1i("velocityField", 1);
    gr_computeAdvection.uniform1i("outputField",   2);
    gr_computeAdvection.uniform1f("ku_dt",            g_dt);
    gr_computeAdvection.uniform2iv("ku_simdims",      g_dims.begin());
    gr_computeAdvection.uniform2fv("ku_simUnitCoord", g_simUnitCoords.begin());
    gl::glBindTextureUnit(0, previousIteration);
    gl::glBindTextureUnit(1, previousIteration);
    gl::glBindImageTexture(1, gr_outTexShader1, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    gl::glDispatchCompute(g_dims.x, g_dims.y, 1);


    /* Compute diffuse component of the velocity field to diffusion texture (gr_outTexShader2) */
    u32 textureInput, textureOutput;
    gr_computeDiffusionVel.bind();
    gr_computeDiffusionVel.uniform1i("initialField", 0);
    gr_computeDiffusionVel.uniform1i("outputField",  1);
    gr_computeDiffusionVel.uniform1i("ku_valuesToFetch", g_reductionFactor);
    gr_computeDiffusionVel.uniform1f("ku_dt",            g_dt);
    gr_computeDiffusionVel.uniform1f("ku_viscosity",     g_viscosity);
    gr_computeDiffusionVel.uniform2iv("ku_simdims",      g_dims.begin());
    gr_computeDiffusionVel.uniform2fv("ku_simUnitCoord", g_simUnitCoords.begin());
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


    gr_computeDivergence.bind();
    gr_computeDivergence.uniform1i("intermediateVelocityField", 0);
    gr_computeDivergence.uniform1i("oldPressureFieldValue",     1);
    gr_computeDivergence.uniform1i("outputField",               2);
    gr_computeDivergence.uniform2fv("ku_simUnitCoord", g_simUnitCoords.begin());
    gl::glBindTextureUnit(0, gr_outTexShader3);
    gl::glBindTextureUnit(1, previousIteration);
    gl::glBindImageTexture(2, gr_outTexShader4, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    gl::glDispatchCompute(g_dims.x, g_dims.y, 1);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);



    /* Compute the new pressure to .z component (gr_outTexShader5) */
    gr_computeDiffusionPressure.bind();
    gr_computeDiffusionPressure.uniform1i("oldPressureField", 0);
    gr_computeDiffusionPressure.uniform1i("newPressureField", 1);
    gr_computeDiffusionPressure.uniform2iv("ku_simdims", g_dims.begin());
    gr_computeDiffusionPressure.uniform2fv("ku_simUnitCoord", g_simUnitCoords.begin());
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
    // gl::glBindImageTexture(1, gr_outTexShader5, 0, false, 0, 
    //     gl::GL_WRITE_ONLY, 
    //     gl::GL_RGBA32F
    // );
    // gl::glDispatchCompute(g_dims.x, g_dims.y, 1);
    // gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);


    /* add together all the shit we computed for the next iteration */
    gr_computeNewVelocity.bind();
    gr_computeNewVelocity.uniform1i("intermediate",    0);
    gr_computeNewVelocity.uniform1i("updatedFields",   1);
    gr_computeNewVelocity.uniform2fv("ku_simUnitCoord", g_simUnitCoords.begin());
    gl::glBindTextureUnit(0, gr_outTexShader5);
    gl::glBindImageTexture(1, nextIteration, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    gl::glDispatchCompute(g_dims.x, g_dims.y, 1);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
    return;
}


static void compute_dye(
    notused u32 velocityPressureField,
    notused u32 previousIterationDye, 
    notused u32 nextIterationDye
) {
    return;
}


static void compute_cfl(u32 texture)
{
    auto jamAFenceToWaitForGPUBufferVisibility = []() {
        if(g_fence) {
            gl::glDeleteSync(g_fence);
        }
        g_fence = gl::glFenceSync(gl::GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        return;
    };
    gl::glBindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, 1, g_reductionMaxTexture);
    gl::glBindTextureUnit(0, texture);
    gr_computeCFLCondition.bind();
    gr_computeCFLCondition.uniform1i("originalTexture", 0);
    gr_computeCFLCondition.StorageBlock("reductionMaximumBuffer", 1);
    gr_computeCFLCondition.uniform1i("ku_valuesToFetch", g_reductionFactor);
    gl::glDispatchCompute(g_reductionBufferLength, 1, 1);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
    

    jamAFenceToWaitForGPUBufferVisibility();


    g_retrieveTextureData.begin();
    // static char static_charbuf[512]{0};
    // if(g_mousePressed && awc2isMouseButtonPressed(AWC2_MOUSEBUTTON_RIGHT)) {
    //     for(i32 i = 0; i < g_reductionBufferLength; ++i) {
    //         mappedptr[i].to_strbuf(&static_charbuf[0], 512);
    //         printf("  %u => %s\n", i, static_charbuf);
    //     }
    // }

    bool biggerX, biggerY;
    auto* mappedptr = __rcast(vec4f*, g_reductionMaxMappedBuf);
    for(i32 i = 0; i < g_reductionBufferLength; ++i) {
        biggerX = boolean(mappedptr[i].x > g_velocityCFL.x);
        biggerY = boolean(mappedptr[i].y > g_velocityCFL.y);
        g_velocityCFL.x = __scast(f32, biggerX) * mappedptr[i].x + __scast(f32, !biggerX) * g_velocityCFL.x;
        g_velocityCFL.y = __scast(f32, biggerY) * mappedptr[i].y + __scast(f32, !biggerY) * g_velocityCFL.y;
    }
    g_retrieveTextureData.end();
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