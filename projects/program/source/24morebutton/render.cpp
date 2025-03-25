#include "render.hpp"
#include <util/random.hpp>
#include <awc2/C/awc2.h>
#include "util/time.hpp"
#include "vars.hpp"
#include "backend24.hpp"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>


using namespace morebutton24;


static void render_imgui_interface_new();


void morebutton24::render()
{
    static const vec4f defaultScreenColor = vec4f{1.0f, 1.0f, 1.0f, 1.0f}; 


    gl::glClearNamedFramebufferfv( /* Clear Screen */
        g_fbo, 
        gl::GL_COLOR, 
        0, 
        defaultScreenColor.begin()
    );


    TIME_NAMESPACE_TIME_CODE_BLOCK(g_renderImguiTime, render_imgui_interface_new());

    /* literally just the velocity texture on the screen */
    // if(g_slowRender) {
    //     if(g_measureTimeOnce) {
    //         g_waitTime = 
    //             g_slowRenderWaitDurationNs 
    //             - g_computeFluidTime.previous_value().count() 
    //             - g_renderScreenTime.previous_value().count();

    //         g_measureTimeOnce = false;
    //         g_skipRender = true;
    //     }


    //     g_waitTime -= g_frameTime.previous_value().count();
    //     if(g_waitTime <= 0) {
    //         g_skipRender = false;
    //         g_measureTimeOnce = true;
    //     }
    // } else {
    //     g_skipRender      = false;
    //     g_measureTimeOnce = true;
    // }


    // if(g_skipRender)
    //     return;


    u32 texToRender = 0;
    TIME_NAMESPACE_TIME_CODE_BLOCK(g_computeFluidTime, texToRender = compute_fluid());
    g_renderScreenTime.begin();
    gr_computeRenderToTex.bind();
    gr_computeRenderToTex.uniform1i("fieldSampler", 0);
    gr_computeRenderToTex.uniform1i("screentexture", 1);
    gr_computeRenderToTex.uniform1ui("ku_selectTextureDraw", g_chooseTextureToRender);
    gr_computeRenderToTex.uniform1f("ku_brightness",         g_textureHighlightSmallValue);
    gr_computeRenderToTex.uniform2iv("ku_simdims",           g_dims.begin());
    gr_computeRenderToTex.uniform2fv("ku_simUnitCoord",      g_simUnitCoords.begin());
    gl::glBindTextureUnit(0, texToRender);
    gl::glBindImageTexture(1, gr_drawTexture, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    gl::glDispatchCompute(g_computeInvocationSize.x, g_computeInvocationSize.y, g_computeInvocationSize.z);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);



    /* Draw Call */
    gl::glBlitNamedFramebuffer(g_fbo, 0, 
        0, 0, g_dims.x,     g_dims.y, 
        0, 0, g_windowSize.x, g_windowSize.y,
        gl::GL_COLOR_BUFFER_BIT, 
        gl::GL_NEAREST
    );
    g_renderScreenTime.end();


    return;
}


static void render_imgui_interface_new()
{
    static vec2f tmp = g_maxVelocity;
    static i64 frameTimeNs;
    static constexpr const char* imgui_diagnostics_text = "\
FramesRendered FrameTime =>    min       max         avg\n\
%-8u       %-6.4f ms      %-10.4f%-10.4f  %-10.4f\n\
Window Size    Simulation Size  Mouse (%u) Position       (%8.4f, %8.4f)\n\
(%-4d, %-4d)   (%-4d, %-4d)     Mouse (%u) Dragging Force (%8.4f, %8.4f)\n\
Normalized Timestep CFL (< 1 )  Maximum Velocity\n\
%-9.6f           %-9.6f   (%-8.4f, %-8.4f)\n\
Reynolds Number\n%-9.6f\n\
Simulation Error (u.x, u.y, p, 0) \n\
Min (%-8.4f, %-8.4f, %-8.4f, %-8.4f)\n\
Max (%-8.4f, %-8.4f, %-8.4f, %-8.4f)\n\
Avg (%-8.4f, %-8.4f, %-8.4f, %-8.4f)\n\
Approximate Maximal Spectral Radius: %-8.4f\n\
Approximate         Iteration Error: %-8.4f\n\
\
Compute Time\n\
%-6.4f [render]\n\
- %-6.4f [fluid]\n\
    - %-6.4f [velocity]\n\
    - %-6.4f [dye]\n\
    - %-6.4f [cfl]\n\
        - %-6.4f [gpu_side]\n\
        - %-6.4f [cpu_side]\n\
    - %6.4f  [err]\n\
        - %6.4f  [gpu_side]\n\
        - %6.4f  [cpu_side]\n\
- %-6.4f [render_imgui]\n\
- %-6.4f [compute_screen+blit]\n\
";

    static f32 timeMeasurements[16] = {0};
    f32& __wholeFrame   = timeMeasurements[0] = g_frameTime.value_units<f32>(1000);
    f32& __rendering    = timeMeasurements[1] = g_renderTime.value_units<f32>(1000);
    f32& __computefluid = timeMeasurements[2] = g_computeFluidTime.value_units<f32>(1000);
    f32& __computevel   = timeMeasurements[3] = g_computeVelTime.value_units<f32>(1000);
    f32& __computedye   = timeMeasurements[4] = g_computeDyeTime.value_units<f32>(1000);
    f32& __computecfl   = timeMeasurements[5] = g_computeCFLTime.value_units<f32>(1000);
    f32& __cflgpuside   = timeMeasurements[6] = g_computeMaximumCPU.value_units<f32>(1000);
    f32& __cflcpuside   = timeMeasurements[7] = g_computeMaximumGPU.value_units<f32>(1000);
    f32& __computeerr   = timeMeasurements[8] = g_computeErrEstimateTime.value_units<f32>(1000);
    f32& __errgpuside   = timeMeasurements[8] = g_computeErrorGPU.value_units<f32>(1000);
    f32& __errcpuside   = timeMeasurements[9] = g_computeErrorCPU.value_units<f32>(1000);
    f32& __imgui_render = timeMeasurements[10] = g_renderImguiTime.value_units<f32>(1000);
    f32& __imgui_screen = timeMeasurements[11] = g_renderScreenTime.value_units<f32>(1000);
    frameTimeNs         = g_frameTime.previous_value().count();
    g_maxFrameTimeNs = (g_maxFrameTimeNs > frameTimeNs) ? g_maxFrameTimeNs : frameTimeNs;
    g_minFrameTimeNs = (g_minFrameTimeNs < frameTimeNs) ? g_minFrameTimeNs : frameTimeNs;
    g_avgFrameTimeNs += frameTimeNs;


    ImGui::Begin("ImGui Interaction Window");
    if(ImGui::Button("Restart Simulation")) {
        for(auto& tex : g_texture) {
            gl::glClearTexImage(tex, 0, gl::GL_RGBA, gl::GL_FLOAT, nullptr);
        }
    }
    ImGui::SameLine();
    if(ImGui::Button("Reset Min-Max-Avg")) {
        g_maxFrameTimeNs = 0;
        g_minFrameTimeNs = 1'000'000'000ll;
        g_avgFrameTimeNs = 0;
    }
    ImGui::SameLine();
    if(ImGui::Checkbox("Lock Source", &g_mouseLockInPlace)) {
        g_mousePressOverride = g_mouseLockInPlace;
    }
    ImGui::SameLine();
    ImGui::Checkbox("Use Vorticity", &g_useVorticityConfinement);
    ImGui::Text("Input Method\n");
    ImGui::PushID("ImGuiCheckBoxInputMethodIndex9Dye");
    if(ImGui::Checkbox("Dye", &g_imGuiButton[9])) {
        if(g_imGuiButton[9]) g_imGuiButton[8] = 0;
        g_chooseUserDrawFillType = g_imGuiButton[9] ? USER_DRAW_FILL_TYPE_DYE : g_chooseUserDrawFillType;
    }
    ImGui::PopID();
    ImGui::SameLine();
    if(ImGui::Checkbox("Force", &g_imGuiButton[8])) { 
        if(g_imGuiButton[8]) g_imGuiButton[9] = 0;
        g_chooseUserDrawFillType = g_imGuiButton[8] ? USER_DRAW_FILL_TYPE_FORCE : g_chooseUserDrawFillType;
    }
    ImGui::SameLine();
    if(ImGui::Checkbox("Force And Dye", &g_imGuiButton[7])) { 
        g_imGuiButton[9] = g_imGuiButton[7];
        g_imGuiButton[8] = g_imGuiButton[7];
        g_chooseUserDrawFillType = g_imGuiButton[7] ? USER_DRAW_FILL_TYPE_FORCE_AND_DYE : g_chooseUserDrawFillType;
    }
    // ImGui::SameLine();
    // if(ImGui::Checkbox("Boundary", &g_imGuiButton[7]))
    // {
    //     if(g_imGuiButton[7]) {
    //         g_imGuiButton[5] = 0;
    //         g_imGuiButton[6] = 0;
    //     }
    //     g_chooseUserDrawFillType = g_imGuiButton[7] ? 2 : g_chooseUserDrawFillType;
    // }

    ImGui::Text("Visualization");
    if(ImGui::Checkbox("Velocity", &g_imGuiButton[0])) {
        if(g_imGuiButton[0]) {
            g_imGuiButton[1] = 0;
            g_imGuiButton[2] = 0;
            g_imGuiButton[3] = 0;
            g_imGuiButton[4] = 0;
            g_imGuiButton[5] = 0;
        }
        markfmt("Chose %s", "Velocity Texture");
        g_chooseTextureToRender = g_imGuiButton[0] ? TEXTURE_DRAW_VELOCITY_PRESSURE : g_chooseTextureToRender;
    }
    ImGui::SameLine();    
    ImGui::PushID("ImGuiCheckBoxDrawTextureTypeIndex1Dye");
    if(ImGui::Checkbox("Dye", &g_imGuiButton[1])) {
        if(g_imGuiButton[1]) {
            g_imGuiButton[0] = 0;
            g_imGuiButton[2] = 0;
            g_imGuiButton[3] = 0;
            g_imGuiButton[4] = 0;
            g_imGuiButton[5] = 0;
        }
        markfmt("Chose %s", "Dye Texture");
        g_chooseTextureToRender = g_imGuiButton[1] ? TEXTURE_DRAW_DYE : g_chooseTextureToRender;
    }
    ImGui::PopID();
    ImGui::SameLine();
    if(ImGui::Checkbox("Curl", &g_imGuiButton[2])) {
        if(g_imGuiButton[2]) {
            g_imGuiButton[0] = 0;
            g_imGuiButton[1] = 0;
            g_imGuiButton[3] = 0;
            g_imGuiButton[4] = 0;
            g_imGuiButton[5] = 0;
        }
        markfmt("Chose %s", "Curl Texture");
        g_chooseTextureToRender = g_imGuiButton[2] ? TEXTURE_DRAW_CURL : g_chooseTextureToRender;
    }
    ImGui::SameLine();
    if(ImGui::Checkbox("Absolute Curl", &g_imGuiButton[3])) {
        if(g_imGuiButton[3]) {
            g_imGuiButton[0] = 0;
            g_imGuiButton[1] = 0;
            g_imGuiButton[2] = 0;
            g_imGuiButton[4] = 0;
            g_imGuiButton[5] = 0;
        }
        markfmt("Chose %s", "Abs Curl Texture");
        g_chooseTextureToRender = g_imGuiButton[3] ? TEXTURE_DRAW_ABS_CURL : g_chooseTextureToRender;
    }
    ImGui::SameLine();
    if(ImGui::Checkbox("Error (Velocity)", &g_imGuiButton[4])) {
        if(g_imGuiButton[4]) {
            g_imGuiButton[0] = 0;
            g_imGuiButton[1] = 0;
            g_imGuiButton[2] = 0;
            g_imGuiButton[3] = 0;
            g_imGuiButton[5] = 0;
        }
        markfmt("Chose %s", "Error Texture");
        g_chooseTextureToRender = g_imGuiButton[4] ? TEXTURE_DRAW_ERROR_VELOCITY : g_chooseTextureToRender;
    }
    ImGui::SameLine();
    if(ImGui::Checkbox("Error (Pressure)", &g_imGuiButton[5])) {
        if(g_imGuiButton[5]) {
            g_imGuiButton[0] = 0;
            g_imGuiButton[1] = 0;
            g_imGuiButton[2] = 0;
            g_imGuiButton[3] = 0;
            g_imGuiButton[4] = 0;
        }
        markfmt("Chose %s", "Error Texture");
        g_chooseTextureToRender = g_imGuiButton[5] ? TEXTURE_DRAW_ERROR_PRESSURE : g_chooseTextureToRender;
    }


    if (ImGui::BeginTable("Table", 5, ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_BordersInnerV))
    {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Delta Time");
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::PushID("parameter0");
        ImGui::DragFloat("", &g_dt, 0.001f, 0.0001f, 3.0f, "%20.16f");
        ImGui::PopID();


        ImGui::TableSetColumnIndex(1);
        ImGui::Text("splatterRadius");
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::PushID("parameter1");
        ImGui::DragFloat("", &g_splatterRadius, 0.001f, 0.001f, 0.2f, "%9.6f");
        ImGui::PopID();
        
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("Viscosity");
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::PushID("parameter2");
        ImGui::DragFloat("", &g_kinematicViscosity, 0.01f, 0.1f, 10.0f, "%9.6f");
        ImGui::PopID();

        ImGui::TableSetColumnIndex(3);
        ImGui::Text("Pressure Density");
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::PushID("parameter3");
        ImGui::DragFloat("", &g_densityPressure, 0.001f, 0.01f, 10.0f, "%9.6f");
        ImGui::PopID();

        ImGui::TableSetColumnIndex(4);
        ImGui::Text("Vorticity");
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::PushID("parameter4");
        ImGui::DragFloat("", &g_confineVorticity, 0.01f, 0.0f, 40.0f, "%9.6f");
        ImGui::PopID();


        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Diffusion Solver Iterations");
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::PushID("parameter5");
        ImGui::DragInt("", &g_maximumJacobiIterations, 0.5f, 10, 1000, "%d");
        ImGui::PopID();

        ImGui::TableSetColumnIndex(1);
        ImGui::Text("Brightness (Applies to Velocity/AbsCurl)");
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::PushID("parameter6");
        ImGui::DragFloat("", &g_textureHighlightSmallValue, 0.01f, 1.0f, 100.0f, "%6.3f");
        ImGui::PopID();
    
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("Characteristic Length");
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::PushID("parameter7");
        
        if(ImGui::DragFloat("", &g_simUnitCoords.x, 0.01f, 0.01f, 10.0f, "%6.3f"))
            g_simUnitCoords.y = g_simUnitCoords.x;
        ImGui::PopID();


        ImGui::EndTable();
    }
    if(g_mousePressRight) {
        if(g_runCodeOnce[0]) {
            g_splatterColor = vec4f{ random32f(), random32f(), random32f(), 1.0f };
            g_runCodeOnce[0] = true;
        }
    }
    g_runCodeOnce[0] = awc2isMouseButtonReleased(AWC2_MOUSEBUTTON_RIGHT);
    if(g_mouseLockInPlace) {
        g_mousedxdy = vec4f{ 
            std::sinf(2 * pi<f32> * g_dt * g_frameCounter / g_frameTime.value_units<f32>(1)), 
            1.0f, 
            0.0f, 0.0f
        };
    }
    ImGui::Text("splatterForce");
    ImGui::SameLine();
    ImGui::ColorButton("splatterForce", *__rcast(ImVec4*, g_mousedxdy.begin()) );
    ImGui::Text("splatterColor");
    ImGui::SameLine();
    ImGui::ColorButton("splatterColor", *__rcast(ImVec4*, g_splatterColor.begin()) );


    if(frameTimeNs < g_maxFrameTimeNs) {
        g_normdt = g_dt / __wholeFrame;
    }
    g_normdt = g_dt;
    g_cfl = g_normdt * (
        + g_maxVelocity.x / g_simUnitCoords.x
        + g_maxVelocity.y / g_simUnitCoords.y
    );
    ImGui::Text(imgui_diagnostics_text,
        g_frameCounter, __wholeFrame, 
        __scast(f32, g_minFrameTimeNs) * 1e-6, 
        __scast(f32, g_maxFrameTimeNs) * 1e-6,
        __scast(f32, g_avgFrameTimeNs) * 1e-6 / __scast(f32, g_frameCounter),

        g_mousePress, g_mousePosition.x, g_mousePosition.y,
        g_windowSize.x, g_windowSize.y, g_dims.x, g_dims.y, 
        g_mousePress, g_mousedxdy.x, g_mousedxdy.y,
        g_normdt, g_cfl, g_maxVelocity.x, g_maxVelocity.y,
        util::math::dot(g_maxVelocity, g_simUnitCoords) / g_kinematicViscosity,

        g_currErrorValues[0].x, g_currErrorValues[0].y, g_currErrorValues[0].z, g_currErrorValues[0].w,
        g_currErrorValues[1].x, g_currErrorValues[1].y, g_currErrorValues[1].z, g_currErrorValues[1].w,
        g_currErrorValues[2].x, g_currErrorValues[2].y, g_currErrorValues[2].z, g_currErrorValues[2].w,

        g_maxSpectralRadius,
        g_iterationErrorN,

        __rendering,
        __computefluid,
        __computevel,
        __computedye,
        __computecfl,
        __cflcpuside,
        __cflgpuside,
        __computeerr,
        __errgpuside,
        __errcpuside,
        __imgui_render,
        __imgui_screen
    );
    ImGui::End();


    return;
}