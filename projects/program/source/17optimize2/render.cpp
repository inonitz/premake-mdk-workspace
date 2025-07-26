#include "render.hpp"
#include <util2/random.hpp>
#include <util2/C/marker4.h>
#include <awc2/C/awc2.h>
#include <util2/C/marker4.h>
#include "vars.hpp"
#include "backend17.hpp"
#include <imgui/imgui.h>


using namespace optimize17;


static void render_imgui_interface();


void optimize17::render()
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
            comp.resizeLocalWorkGroup(0, 1, 1, 1 );
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
    UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(g_renderImguiTime, render_imgui_interface());
    UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(g_computeFluidTime, texToRender = compute_fluid());


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
            g_splatterColor = vec4f{ util2::random32f(), util2::random32f(), 0.0f, 1.0f };
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


    g_maxRenderTime = (g_maxRenderTime > timeMeasurements[0]) ? g_maxRenderTime : timeMeasurements[0];
    g_minRenderTime = (g_minRenderTime < timeMeasurements[0]) ? g_minRenderTime : timeMeasurements[0];
    g_avgRenderTime += timeMeasurements[0];


    ImGui::Begin("ImGui Interaction Window");
    ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_None);
    
    if (ImGui::BeginTabItem("Simulation Parameters"))
    {
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
        if(ImGui::Button("Reset Min-Max Frame")) {
            g_minRenderTime = 1000.0f;
            g_maxRenderTime = 0.0f;
        }
        ImGui::DragFloat2("Unit Coordinate Size", g_simUnitCoords.begin(), 0.1f, 0.1f, 100.0f);
        ImGui::DragFloat("Ambient Temperature",  &g_ambientTemperature, 0.01f, -40.0f,  40.0f, "%9.6f");
        ImGui::DragFloat("Delta Time",           &g_dt,                 0.001f, 0.001f, 0.5f,  "%9.6f");
        ImGui::DragFloat("Buoyancy ",            &g_buoyancy,           0.01f,  0.1f,   10.0f, "%9.6f");
        ImGui::DragFloat("Viscosity",            &g_viscosity,          0.01f,  0.1f,   10.0f, "%9.6f");
        ImGui::DragFloat("splatterRadius",       &g_splatterRadius,     0.001f, 0.001f, 0.2f,  "%9.6f");
        ImGui::DragInt("Diffusion Solver Iterations", &g_maximumJacobiIterations, 0.5f, 10, 80);
        ImVec4* colorcvt = __rcast(ImVec4*, g_splatterColor.begin());
        ImGui::ColorButton("splatterColor ", *colorcvt);

        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Diagnostics"))
    {
        ImGui::Text("\
Mouse Dragging Force %s\n\
Mouse Position       %s\n\
Mouse was Pressed    %u\n\
CFL Condition (< 1)  %9.6f\n\
FrameCounter         %u (%6.4f ms [min, max, avg] = [ %6.4f, %6.4f, %6.4f ] )\n\
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
",
    dragForceStr, 
    mouseDragPosStr,
    g_mousePressed,
    (cfl.x + cfl.y) * g_dt,
    g_frameCounter,
    timeMeasurements[0],
    g_minRenderTime,
    g_maxRenderTime,
    g_avgRenderTime / __scast(f32, g_frameCounter),
    util2::Time::getGeneralPurposeStamp(0).value_units<f32>(1000),
    util2::Time::getGeneralPurposeStamp(1).value_units<f32>(1000),
    util2::Time::getGeneralPurposeStamp(2).value_units<f32>(1000),
    util2::Time::getGeneralPurposeStamp(3).value_units<f32>(1000),
    util2::Time::getGeneralPurposeStamp(4).value_units<f32>(1000),
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
    maxVelStr
);
        ImGui::EndTabItem();
    }    
    ImGui::EndTabBar();
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