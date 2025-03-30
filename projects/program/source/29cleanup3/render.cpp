#include "render.hpp"
#include "fluid.hpp"
#include "util/macro.h"
#include "vars.hpp"
#include <glbinding/gl/gl.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <awc2/C/awc2.h>


using namespace cleanup329;
using namespace util::math;


void render::clear()
{
    static const vec4f defaultScreenColor = vec4f{1.0f, 1.0f, 1.0f, 1.0f}; 
    gl::glClearNamedFramebufferfv( /* Clear Screen */
        g_fbo, 
        gl::GL_COLOR, 
        0, 
        defaultScreenColor.begin()
    );


    return;
}


void render::render()
{
    AWC2ViewportSize winsize = awc2getCurrentContextViewport();
    /* Draw Call */
    gl::glBlitNamedFramebuffer(g_fbo, 0, 
        0, 0, g_dims.x,               g_dims.y, 
        0, 0, __scast(i32, winsize.x), __scast(i32, winsize.y),
        gl::GL_COLOR_BUFFER_BIT, 
        gl::GL_NEAREST
    );
    return;
}


void render::render_imgui()
{
    static auto winsize = awc2getCurrentContextViewport();
    static vec2f tmp = g_maxVelocity;
    static i64 frameTimeNs;
    static constexpr const char* imgui_diagnostics_text = "\
Window Size    Simulation Size\n\
(%-4d, %-4d)   (%-4d, %-4d)   \n\
%-9.6f Timestep\n\
%-9.6f CFL (< 1)\n\
%-9.6f Reynolds Number\n\
%-9.6f Approximate Maximal Spectral Radius\n\
%-9.6f Approximate         Iteration Error\n\
(%-8.4f, %-8.4f) Maximum Velocity\n\
Simulation Error (u.x, u.y, p, 0) \n\
Min (%-8.4f, %-8.4f, %-8.4f, %-8.4f)\n\
Max (%-8.4f, %-8.4f, %-8.4f, %-8.4f)\n\
Avg (%-8.4f, %-8.4f, %-8.4f, %-8.4f)\n\
FramesRendered FrameTime =>    min       max         avg\n\
%-8u       %-6.4f ms      %-10.4f%-10.4f  %-10.4f\n\
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

    static f32 timeMeasurements[__carraysize(g_timerBuffer)] = {0};
    for(u32 i = 0; i < __carraysize(g_timerBuffer); ++i) {
        timeMeasurements[i] = g_timerBuffer[i].value_units<f32>(1000);
    }
    f32& __wholeFrame   = timeMeasurements[0];
    f32& __rendering    = timeMeasurements[1];
    f32& __computefluid = timeMeasurements[2];
    f32& __computevel   = timeMeasurements[3];
    f32& __computedye   = timeMeasurements[4];
    f32& __computecfl   = timeMeasurements[5];
    f32& __cflgpuside   = timeMeasurements[6];
    f32& __cflcpuside   = timeMeasurements[7];
    f32& __computeerr   = timeMeasurements[8];
    f32& __errgpuside   = timeMeasurements[8];
    f32& __errcpuside   = timeMeasurements[9];
    f32& __imgui_render = timeMeasurements[10];
    f32& __imgui_screen = timeMeasurements[11];
    frameTimeNs         = g_frameTime.previous_value().count();
    g_maxFrameTimeNs = (g_maxFrameTimeNs > frameTimeNs) 
        ? g_maxFrameTimeNs : frameTimeNs;
    g_minFrameTimeNs = (g_minFrameTimeNs < frameTimeNs) 
        ? g_minFrameTimeNs : frameTimeNs;
    g_avgFrameTimeNs += frameTimeNs;


    ImGui::Begin("ImGui Interaction Window");
    if(ImGui::Button("Restart Simulation")) {
        fluid::clear();
    }
    if(ImGui::Button("Recompile Shaders")) {
        fluid::recompileComputeShaders();
    }
    if(ImGui::Button("Reset Min-Max-Avg")) {
        g_maxFrameTimeNs = 0;
        g_minFrameTimeNs = 1'000'000'000ll;
        g_avgFrameTimeNs = 0;
    }
    ImGui::SameLine();


    auto resetButtonsButOne = [](u8 i) {
        memset(g_imGuiButton, 0x00, __carraysize(g_imGuiButton));
        g_imGuiButton[i] = true;
        g_chooseTextureToRender = __scast(DrawTarget, i);
        return;
    };
    auto buttonQuery = [&resetButtonsButOne](
        const char* str, 
        bool* button, 
        u8    idx, 
        bool  use_sameline
    ) {
        if(ImGui::Checkbox(str, button))
            resetButtonsButOne(idx);
        if(use_sameline)
            ImGui::SameLine();
    
        return;
    };


    ImGui::Text("Visualization");
    buttonQuery("Velocity-Pressure", &g_imGuiButton[__scast(u8, DrawTarget::VELOCITY_PRESSURE)], __scast(u8, DrawTarget::VELOCITY_PRESSURE),  true);
    buttonQuery("Dye              ", &g_imGuiButton[__scast(u8, DrawTarget::DYE              )], __scast(u8, DrawTarget::DYE              ), false);
    buttonQuery("Velocity X       ", &g_imGuiButton[__scast(u8, DrawTarget::VELOCITY_X       )], __scast(u8, DrawTarget::VELOCITY_X       ),  true);
    buttonQuery("Velocity Y       ", &g_imGuiButton[__scast(u8, DrawTarget::VELOCITY_Y       )], __scast(u8, DrawTarget::VELOCITY_Y       ), false);
    buttonQuery("Pressure         ", &g_imGuiButton[__scast(u8, DrawTarget::PREESURE         )], __scast(u8, DrawTarget::PREESURE         ),  true);
    buttonQuery("Curl             ", &g_imGuiButton[__scast(u8, DrawTarget::CURL             )], __scast(u8, DrawTarget::CURL             ),  true);
    buttonQuery("Absolute Curl    ", &g_imGuiButton[__scast(u8, DrawTarget::ABS_CURL         )], __scast(u8, DrawTarget::ABS_CURL         ), false);
    buttonQuery("Error (Velocity) ", &g_imGuiButton[__scast(u8, DrawTarget::ERROR_VELOCITY   )], __scast(u8, DrawTarget::ERROR_VELOCITY   ),  true);
    buttonQuery("Error (Pressure) ", &g_imGuiButton[__scast(u8, DrawTarget::ERROR_PRESSURE   )], __scast(u8, DrawTarget::ERROR_PRESSURE   ), false);



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
        ImGui::Text("Viscosity");
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::PushID("parameter1");
        ImGui::DragFloat("", &g_kinematicViscosity, 0.01f, 0.1f, 10.0f, "%9.6f");
        ImGui::PopID();

        ImGui::TableSetColumnIndex(2);
        ImGui::Text("Pressure Density");
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::PushID("parameter2");
        ImGui::DragFloat("", &g_densityPressure, 0.001f, 0.01f, 10.0f, "%9.6f");
        ImGui::PopID();

        ImGui::TableSetColumnIndex(3);
        ImGui::Text("Vorticity");
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::PushID("parameter3");
        ImGui::DragFloat("", &g_confineVorticity, 0.01f, 0.0f, 40.0f, "%9.6f");
        ImGui::PopID();

        ImGui::TableSetColumnIndex(4);
        ImGui::Text("Diffusion Solver Iterations");
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::PushID("parameter4");
        ImGui::DragInt("", &g_maximumJacobiIterations, 0.5f, 10, 1000, "%d");
        ImGui::PopID();


        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Brightness (Applies to Velocity/AbsCurl)");
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::PushID("parameter5");
        ImGui::DragFloat("", &g_textureHighlightSmallValue, 0.01f, 1.0f, 100.0f, "%6.3f");
        ImGui::PopID();
    
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("Characteristic Length");
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::PushID("parameter6");
        

        ImGui::DragFloat("", &g_unitLength, 0.01f, 0.01f, 10.0f, "%6.3f");
        ImGui::PopID();


        ImGui::EndTable();
    }
    // m_mousedxdy = vec4f{ 
    //     std::sinf(2 * pi<f32> * m_dt * m_frameCounter / m_frameTime.value_units<f32>(1)), 
    //     1.0f, 
    //     0.0f, 0.0f
    // };


    if(ImGui::Button("Add Source")) {
        fluid::addSource();
    }
    if (ImGui::TreeNodeEx("Fluid Sources", 
        ImGuiTreeNodeFlags_FramePadding
        |
        ImGuiTreeNodeFlags_DefaultOpen
    )) {
        for (u64 n = 0; n < g_sources.size(); n++)
        {
            auto& source = g_sources[n];


            ImGui::PushID(n);
            if (ImGui::TreeNodeEx("", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Text("%u : ", source.m_enabled);
                ImGui::SameLine();
                if(ImGui::Button("Enable/Disable"))
                    source.m_enabled = !source.m_enabled;


                ImGui::Text("%s : ", source.m_type == FillType::FORCE ? 
                    "FORCE" : source.m_type == FillType::DYE 
                    ? "DYE  " : 
                    "NONE "
                );
                ImGui::SameLine();
                if(ImGui::Button("Force/Dye"))
                    source.m_type = (source.m_type == FillType::DYE) 
                    ? 
                    FillType::FORCE : FillType::DYE;


                ImGui::PushItemWidth(60);
                ImGui::DragFloat("Radius", &source.m_radius, 0.0001f, 0.0001f, 0.5f, "%-8.6f");
                ImGui::PopItemWidth();

                ImGui::Text("Position: ");
                ImGui::SameLine();
                ImGui::PushItemWidth(80);
                ImGui::PushID("DragFloatWinSizeX");
                ImGui::DragFloat("", &source.m_position.x, 0.5f, 0.0f, __scast(f32, winsize.x));
                ImGui::PopID();
                ImGui::PopItemWidth();
                ImGui::SameLine();
                ImGui::PushItemWidth(80);
                ImGui::PushID("DragFloatWinSizeY");
                ImGui::DragFloat("", &source.m_position.y, 0.5f, 0.0f, __scast(f32, winsize.y));
                ImGui::PopID();
                ImGui::PopItemWidth();

                ImGui::TextColored(*__rcast(ImVec4*, &source.m_color), "Color/Value: ( %-9.6f, %-9.6f, %-9.6f, %-9.6f )\n",
                    source.m_color.x, 
                    source.m_color.y, 
                    source.m_color.z, 
                    source.m_color.w
                );


                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        ImGui::TreePop();
    }




    if(frameTimeNs < g_maxFrameTimeNs) {
        g_normdt = g_dt / __wholeFrame;
    }
    g_normdt = g_dt;
    g_cfl = g_normdt * (
        + g_maxVelocity.x / g_unitLength
        + g_maxVelocity.y / g_unitLength
    );
    

    vec2f tmp0 = g_maxVelocity * (g_unitLength / g_kinematicViscosity);
    g_reynolds = tmp0.x + tmp0.y;


    ImGui::Text(imgui_diagnostics_text,
        winsize.x, winsize.y, g_dims.x, g_dims.y,

        g_normdt,
        g_cfl,
        g_reynolds,
        g_maxSpectralRadius,
        g_iterationErrorN,
        g_maxVelocity.x, g_maxVelocity.y,

        g_currErrorValues[0].x, g_currErrorValues[0].y, 
        g_currErrorValues[0].z, g_currErrorValues[0].w,
        g_currErrorValues[1].x, g_currErrorValues[1].y, 
        g_currErrorValues[1].z, g_currErrorValues[1].w,
        g_currErrorValues[2].x, g_currErrorValues[2].y, 
        g_currErrorValues[2].z, g_currErrorValues[2].w,

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
        __computeerr,
        __errgpuside,
        __errcpuside,
        __imgui_render,
        __imgui_screen
    );
    ImGui::End();


    return;
}