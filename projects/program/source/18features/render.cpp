#include "render.hpp"
#include <util2/random.hpp>
#include <awc2/C/awc2.h>
#include <util2/C/marker4.h>
#include <util2/time.hpp>
#include "vars.hpp"
#include "backend18.hpp"
#include <imgui/imgui.h>


using namespace features18;


static void render_imgui_interface();


void features18::render()
{
    static const vec4f defaultScreenColor = vec4f{1.0f, 1.0f, 1.0f, 1.0f}; 


    gl::glClearNamedFramebufferfv( /* Clear Screen */
        g_fbo, 
        gl::GL_COLOR, 
        0, 
        defaultScreenColor.begin()
    );


    UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(g_renderImguiTime, render_imgui_interface());

    /* literally just the velocity texture on the screen */
    if(g_slowRender) {
        if(g_measureTimeOnce) {
            g_waitTime = 
                g_slowRenderWaitDurationNs 
                - g_computeFluidTime.previous_value().count() 
                - g_renderScreenTime.previous_value().count();

            g_measureTimeOnce = false;
            g_skipRender = true;
        }


        g_waitTime -= g_frameTime.previous_value().count();
        if(g_waitTime <= 0) {
            g_skipRender = false;
            g_measureTimeOnce = true;
        }
    } else {
        g_skipRender      = false;
        g_measureTimeOnce = true;
    }


    if(g_skipRender)
        return;


    u32 texToRender = 0;
    UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(g_computeFluidTime, texToRender = compute_fluid());
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
    gl::glDispatchCompute(g_computeInvocationSize.x, g_computeInvocationSize.y, g_computeInvocationSize.z);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);



    /* Draw Call */
    // gl::glNamedFramebufferTexture(g_fbo, gl::GL_COLOR_ATTACHMENT0, g_texture[1], 0);
    gl::glBlitNamedFramebuffer(g_fbo, 0, 
        0, 0, g_dims.x,     g_dims.y, 
        0, 0, g_windims.x, g_windims.y,
        gl::GL_COLOR_BUFFER_BIT, 
        gl::GL_NEAREST
    );
    g_renderScreenTime.end();


    return;
}


static void render_imgui_interface()
{
    vec2f tmp;
    f32   cfl;
    char* dragForceStr, *mouseDragPosStr;
    const auto currentWindowSize = awc2getCurrentContextViewport();
    static f32 timeMeasurements[16] = {0};
    static auto refreshShadersFromFiles = []() -> bool {
        u8 shadersGood = 1;

        /* no point updating 9 shaders every time, refactor */
        g_refreshShaderTime.begin();
        for(uint32_t i = 0; shadersGood && i < __carraysize(computeShaderFilename); ++i) {
            g_compute[i].refreshFromFiles();
            g_compute[i].resizeLocalWorkGroup(0,
                g_localWorkGroupSize.x,
                g_localWorkGroupSize.y,
                g_localWorkGroupSize.z
            );
            shadersGood = shadersGood && g_compute[i].compile();
        }
        g_refreshShaderTime.end();


        return shadersGood;
    };
    
    g_mousePressed = false;
    g_windims      = vec2i{ currentWindowSize.x, currentWindowSize.y };


    if(awc2isMouseButtonPressed(AWC2_MOUSEBUTTON_LEFT)) {
        AWC2CursorPosition delta = awc2getMousePositionDelta();
        AWC2CursorPosition pos = awc2getMousePosition();

        g_mouseDragForce    = vec2f{ delta.x, delta.y };
        g_mouseDragPosition = vec2f{ pos.x, pos.y };
        g_mousePressed      = true;
    }
    if(awc2isMouseButtonPressed(AWC2_MOUSEBUTTON_RIGHT)) {
        if(g_runCodeOnceFlag) {
            g_splatterForce = vec4f{ util2::random32f(), util2::random32f(), 0.0f,        1.0f };
            g_splatterColor = vec4f{ util2::random32f(), util2::random32f(), util2::random32f(), 1.0f };
            g_runCodeOnceFlag = false;
        }
    }
    g_runCodeOnceFlag = awc2isMouseButtonReleased(AWC2_MOUSEBUTTON_RIGHT);


    tmp = g_maxVelocity / g_simUnitCoords;
    cfl = (tmp.x + tmp.y) * g_dt;
    dragForceStr    = g_mouseDragForce.to_string();
    mouseDragPosStr = g_mouseDragPosition.to_string();
    f32& __wholeFrame   = timeMeasurements[0];
    f32& __rendering    = timeMeasurements[1];
    f32& __computefluid = timeMeasurements[2];
    f32& __computevel   = timeMeasurements[3];
    f32& __computedye   = timeMeasurements[4];
    f32& __computecfl   = timeMeasurements[5];
    f32& __cflgpuside   = timeMeasurements[6];
    f32& __cflcpuside   = timeMeasurements[7];
    f32& __ref_shader   = timeMeasurements[8];
    f32& __imgui_render = timeMeasurements[9];
    f32& __imgui_screen = timeMeasurements[10];

    __wholeFrame   = g_frameTime.value_units<f32>(1000);
    __rendering    = g_renderTime.value_units<f32>(1000);
    __computefluid = g_computeFluidTime.value_units<f32>(1000);
    __computevel   = g_computeVelTime.value_units<f32>(1000); 
    __computedye   = g_computeDyeTime.value_units<f32>(1000);
    __computecfl   = g_computeCFLTime.value_units<f32>(1000);
    __cflgpuside   = g_computeMaximumCPU.value_units<f32>(1000);
    __cflcpuside   = g_computeMaximumGPU.value_units<f32>(1000);
    __ref_shader   = g_refreshShaderTime.value_units<f32>(1000);
    __imgui_render = g_renderImguiTime.value_units<f32>(1000);
    __imgui_screen = g_renderScreenTime.value_units<f32>(1000);
    timeMeasurements[12] = g_measuremisc[0].value_units<f32>(1000);
    timeMeasurements[13] = g_measuremisc[1].value_units<f32>(1000);


    i64 frameTimeNs  = g_frameTime.previous_value().count();
    g_maxFrameTimeNs = (g_maxFrameTimeNs > frameTimeNs) ? g_maxFrameTimeNs : frameTimeNs;
    g_minFrameTimeNs = (g_minFrameTimeNs < frameTimeNs) ? g_minFrameTimeNs : frameTimeNs;
    g_avgFrameTimeNs += frameTimeNs;


    ImGui::Begin("ImGui Interaction Window");
    ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_None);
    if (ImGui::BeginTabItem("Simulation Parameters"))
    {
        if(ImGui::Button("Slow Render")) {
            g_slowRender = boolean((g_slowRenderCounter % 2) == 0);
            ++g_slowRenderCounter;
        }
        ImGui::SameLine();
        if(ImGui::Button("Restart Simulation")) {
            for(auto& tex : g_texture) {
                gl::glClearTexImage(tex, 0, gl::GL_RGBA, gl::GL_FLOAT, nullptr);
            }
        }
        ImGui::SameLine();
        if(ImGui::Button("Refresh Shaders")) {
            bool canRender = refreshShadersFromFiles();
            g_skipRender = g_skipRender || !canRender;
        }
        ImGui::SameLine();
        if(ImGui::Button("Reset Min-Max Frame")) {
            g_minFrameTimeNs = 1'000'000'000'000;
            g_maxFrameTimeNs = 0;
        }
        if(ImGui::Checkbox("Render Velocity Texture", &g_buttonPressed[0])) {
            if(g_buttonPressed[0]) g_buttonPressed[1] = 0;
            g_whatToRender = g_buttonPressed[0] ? 1 : g_whatToRender;
        }
        if(ImGui::Checkbox("Render Dye      Texture", &g_buttonPressed[1])) {
            if(g_buttonPressed[1]) g_buttonPressed[0] = 0;
            g_whatToRender = g_buttonPressed[1] ? 2 : g_whatToRender;
        }
        if(ImGui::Checkbox("Vorticity Confinement", &g_buttonPressed[2])) {
            g_useVorticityConfinement = g_buttonPressed[1];
        }
        ImGui::DragFloat2("Unit Coordinate Size", g_simUnitCoords.begin(), 0.1f, 0.1f, 100.0f);
        ImGui::DragFloat("Ambient Temperature",  &g_ambientTemperature,   0.01f, -40.0f,  40.0f, "%9.6f");
        ImGui::DragFloat("Delta Time",           &g_dt,                   0.001f, 0.001f, 2.0f,  "%20.16f");
        ImGui::DragFloat("Buoyancy ",            &g_buoyancy,             0.01f,  0.1f,   10.0f, "%9.6f");
        ImGui::DragFloat("Viscosity",            &g_viscosity,            0.01f,  0.1f,   10.0f, "%9.6f");
        ImGui::DragFloat("Vorticity",            &g_vorticityConfinement, 0.01f,  0.0f,   40.0f, "%9.6f");
        ImGui::DragFloat("Damping Factor",       &g_dampFactor,           0.001f, 0.001f, 20.0f, "%9.6f");
        ImGui::DragFloat("splatterRadius",       &g_splatterRadius,       0.001f, 0.001f, 0.2f,  "%9.6f");
        ImGui::DragInt("Diffusion Solver Iterations", &g_maximumJacobiIterations, 0.5f, 10, 1000);
        ImVec4* cvtptr[2] = {
            __rcast(ImVec4*, g_splatterForce.begin()),
            __rcast(ImVec4*, g_splatterColor.begin())
        };
        ImGui::ColorButton("splatterForce ", *cvtptr[0]);
        ImGui::SameLine();
        ImGui::ColorButton("splatterColor ", *cvtptr[1]);
        g_normdt = g_dt / __wholeFrame;

        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Diagnostics"))
    {
        ImGui::Text("\
Window     (%d, %d)\n\
Simulation (%d, %d)\n\
Mouse (Pressed = %u)\n\
    - Dragging Force %s\n\
    - Position       %s\n\
CFL Condition (< 1)  %9.6f (%6.4f, %6.4f) <=> Maximum Velocity\n\
Normalized Timestep  %9.6f\n\
FrameCounter         %u (%6.4f ms [min, max, avg] = [ %6.4f, %6.4f, %6.4f ] )\n\
Compute Time\n\
    %6.4f [render]\n\
    - %6.4f [fluid]\n\
        - %6.4f [velocity]\n\
        - %6.4f [dye]\n\
        - %6.4f [cfl]\n\
            - %6.4f [gpu_side]\n\
            - %6.4f [cpu_side]\n\
    - %6.4f [shader_refresh]\n\
    - %6.4f [render_imgui]\n\
    - %6.4f [compute_screen+blit]\n\
",
    g_windims.x, g_windims.y,
    g_dims.x, g_dims.y,
    g_mousePressed, dragForceStr, mouseDragPosStr,
    cfl, g_maxVelocity.x, g_maxVelocity.y,
    g_normdt,
    g_frameCounter, __wholeFrame, 
    __scast(f32, g_minFrameTimeNs) * 1e-6, 
    __scast(f32, g_maxFrameTimeNs) * 1e-6,
    __scast(f32, g_avgFrameTimeNs) * 1e-6 / __scast(f32, g_frameCounter),
    __rendering,
    __computefluid,
    __computevel,
    __computedye,
    __computecfl,
    __cflcpuside,
    __cflgpuside,
    __ref_shader,
    __imgui_render,
    __imgui_screen
);
        ImGui::EndTabItem();
    }    
    ImGui::EndTabBar();
    ImGui::End();

    free(mouseDragPosStr);
    free(dragForceStr);

    /* 
        g_maxVelocity is from the previous frame. 
        if I reset it after calculating it at compute_cfl() 
        then render_imgui_interface() won't have a chance to render it =>
        after I render the value, I zero it because it's only used for debug purposes now.
    */
    // if(g_maxVelocity.x != 0 && g_maxVelocity.y != 0) {
    //     g_dt = (g_simUnitCoords.x / g_maxVelocity.x) + (g_simUnitCoords.y / g_maxVelocity.y);
    //     g_dt *= 5; 
    // }
    g_maxVelocity = vec2f{0.0f};
    return;
}