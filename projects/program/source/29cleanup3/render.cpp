#include "render.hpp"
#include "fluid.hpp"
#include "util/macro.h"
#include "vars.hpp"
#include <util/random.hpp>
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


static void PushStyleCompact()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::PushStyleVarY(ImGuiStyleVar_FramePadding, (float)(int)(style.FramePadding.y * 0.60f));
    ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, (float)(int)(style.ItemSpacing.y * 0.60f));
}

static void PopStyleCompact()
{
    ImGui::PopStyleVar(2);
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
Compute Time (CPU)\n\
- %-6.4f [begin_frame]\n\
- %-6.4f [fluid_update]\n\
    - %-6.4f [vel]\n\
    - %-6.4f [dye]\n\
    - %-6.4f [cfl]\n\
        - %-6.4f [gpu_side]\n\
        - %-6.4f [cpu_side]\n\
    - %-6.4f [err]\n\
        - %-6.4f [gpu_side]\n\
        - %-6.4f [cpu_side]\n\
    - %-6.4f [postprocess]\n\
- %-6.4f [render_imgui]\n\
- %-6.4f [render_blit]\n\
- %-6.4f [end_frame]\n\
Compute Time (GPU) (%s)\n\
- %-6.4f [vel_cmdbuf]\n\
- %-6.4f [dye_cmdbuf]\n\
- %-6.4f [cfl_cmdbuf]\n\
- %-6.4f [err_cmdbuf]\n\
- %-6.4f [render]\n\
";

    static f32 timeMeasurements[__carraysize(g_cpuTimerBuffer)] = {0};
    for(u32 i = 0; i < __carraysize(g_cpuTimerBuffer); ++i) {
        timeMeasurements[i] = g_cpuTimerBuffer[i].value_units<f32>(1000);
    }
    f32& __frameTime              = timeMeasurements[0];
    f32& __beginFrameTime         = timeMeasurements[1];
    f32& __fluidUpdateTime        = timeMeasurements[2];
    f32& __computeVelTime         = timeMeasurements[3];
    f32& __computeDyeTime         = timeMeasurements[4];
    f32& __computeCFLTime         = timeMeasurements[5];
    f32& __computeErrEstimateTime = timeMeasurements[6];
    f32& __renderScreenTime       = timeMeasurements[7];
    f32& __renderImGuiTime        = timeMeasurements[8];
    f32& __renderBlitTime         = timeMeasurements[9];
    f32& __endFrameTime           = timeMeasurements[10];
    f32& __computeMaximumCPU      = timeMeasurements[11];
    f32& __computeMaximumGPU      = timeMeasurements[12];
    f32& __computeErrorGPU        = timeMeasurements[13];
    f32& __computeErrorCPU        = timeMeasurements[14];
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
    ImGui::SameLine();
    if(ImGui::Button("Recompile Shaders")) {
        fluid::recompileComputeShaders();
    }
    ImGui::SameLine();
    if(ImGui::Button("Reset Min-Max-Avg")) {
        g_maxFrameTimeNs = 0;
        g_minFrameTimeNs = 1'000'000'000ll;
        g_avgFrameTimeNs = 0;
    }
    ImGui::SameLine();
    if(ImGui::Button("Enable/Disable GPU Timers")) {
        g_enableGPUTimers = !g_enableGPUTimers;
    }


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


    ImGui::SeparatorText("Visualization");
    buttonQuery("Velocity-Pressure", &g_imGuiButton[__scast(u8, DrawTarget::VELOCITY_PRESSURE)], __scast(u8, DrawTarget::VELOCITY_PRESSURE),  true);
    buttonQuery("Dye              ", &g_imGuiButton[__scast(u8, DrawTarget::DYE              )], __scast(u8, DrawTarget::DYE              ), false);
    buttonQuery("Velocity X       ", &g_imGuiButton[__scast(u8, DrawTarget::VELOCITY_X       )], __scast(u8, DrawTarget::VELOCITY_X       ),  true);
    buttonQuery("Velocity Y       ", &g_imGuiButton[__scast(u8, DrawTarget::VELOCITY_Y       )], __scast(u8, DrawTarget::VELOCITY_Y       ), false);
    buttonQuery("Pressure         ", &g_imGuiButton[__scast(u8, DrawTarget::PREESURE         )], __scast(u8, DrawTarget::PREESURE         ),  true);
    buttonQuery("CFL Contour      ", &g_imGuiButton[__scast(u8, DrawTarget::CFL_CONTOUR      )], __scast(u8, DrawTarget::CFL_CONTOUR      ), false);
    buttonQuery("Curl             ", &g_imGuiButton[__scast(u8, DrawTarget::CURL             )], __scast(u8, DrawTarget::CURL             ),  true);
    buttonQuery("Absolute Curl    ", &g_imGuiButton[__scast(u8, DrawTarget::ABS_CURL         )], __scast(u8, DrawTarget::ABS_CURL         ), false);
    buttonQuery("Error (Velocity) ", &g_imGuiButton[__scast(u8, DrawTarget::ERROR_VELOCITY   )], __scast(u8, DrawTarget::ERROR_VELOCITY   ),  true);
    buttonQuery("Error (Pressure) ", &g_imGuiButton[__scast(u8, DrawTarget::ERROR_PRESSURE   )], __scast(u8, DrawTarget::ERROR_PRESSURE   ), false);


    ImGui::SeparatorText("Colormap");
    if(ImGui::SmallButton("Inferno")) g_chooseColourMap = ColourMap::INFERNO;
    ImGui::SameLine();
    if(ImGui::SmallButton("Virdis")) g_chooseColourMap = ColourMap::VIRDIS;
    ImGui::SameLine();
    if(ImGui::SmallButton("Fast")) g_chooseColourMap = ColourMap::FAST;
    

    ImGui::Text("Value Range");
    ImGui::SameLine();
    ImGui::PushItemWidth(120);
    ImGui::PushID("DragFloat2ColorTableMinMax");
    ImGui::DragFloat2("", g_colorTableMinMax.begin(), 0.01f, -10.0f, 10.0f, "%6.3f");
    ImGui::PopID();
    ImGui::PopItemWidth();
    
    ImGui::Text("Brightness ");
    ImGui::SameLine();
    ImGui::PushItemWidth(120);
    ImGui::PushID("DragFloatTextureBrightnessFactor");
    ImGui::DragFloat("", &g_textureHighlightSmallValue, 0.01f, 1.0f, 100.0f, "%6.3f");
    ImGui::PopID();
    ImGui::PopItemWidth();   



    ImGui::SeparatorText("Simulation Parameters");
    PushStyleCompact();
    if (ImGui::BeginTable("Table", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX))
    {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Delta Time");
        ImGui::SetNextItemWidth(120);
        ImGui::PushID("parameter0");
        ImGui::DragFloat("", &g_dt, 0.001f, 0.0001f, 3.0f, "%20.16f");
        ImGui::PopID();

        ImGui::TableSetColumnIndex(1);
        ImGui::Text("Viscosity");
        ImGui::SetNextItemWidth(120);
        ImGui::PushID("parameter1");
        ImGui::DragFloat("", &g_kinematicViscosity, 0.01f, 0.1f, 10.0f, "%9.6f");
        ImGui::PopID();

        ImGui::TableSetColumnIndex(2);
        ImGui::Text("Pressure Density");
        ImGui::SetNextItemWidth(120);
        ImGui::PushID("parameter2");
        ImGui::DragFloat("", &g_densityPressure, 0.001f, 0.01f, 10.0f, "%9.6f");
        ImGui::PopID();


        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Vorticity");
        ImGui::SetNextItemWidth(120);
        ImGui::PushID("parameter3");
        ImGui::DragFloat("", &g_confineVorticity, 0.01f, 0.0f, 40.0f, "%9.6f");
        ImGui::PopID();

        ImGui::TableSetColumnIndex(1);
        ImGui::Text("Characteristic Length");
        ImGui::SetNextItemWidth(120);
        ImGui::PushID("parameter4");
        ImGui::DragFloat("", &g_unitLength, 0.01f, 0.01f, 10.0f, "%6.3f");
        ImGui::PopID();

        ImGui::TableSetColumnIndex(2);
        ImGui::Text("Diffusion Solver Iterations");
        ImGui::SetNextItemWidth(120);
        ImGui::PushID("parameter5");
        ImGui::DragInt("", &g_maximumJacobiIterations, 0.5f, 10, 1000, "%d");
        ImGui::PopID();


        ImGui::EndTable();
    }
    PopStyleCompact();
    // m_mousedxdy = vec4f{ 
    //     std::sinf(2 * pi<f32> * m_dt * m_frameCounter / m_frameTime.value_units<f32>(1)), 
    //     1.0f, 
    //     0.0f, 0.0f
    // };


    // ImGui::Text("Gradients");
    // auto* drawList = ImGui::GetWindowDrawList();
    // ImVec2 gradient_size = ImVec2(ImGui::CalcItemWidth(), ImGui::GetFrameHeight());
    // {
    //     ImVec2 p0 = ImGui::GetCursorScreenPos();
    //     ImVec2 p1 = ImVec2(p0.x + gradient_size.x, p0.y + gradient_size.y);
    //     ImU32 col_a = ImGui::GetColorU32(IM_COL32(0, 0, 0, 255));
    //     ImU32 col_b = ImGui::GetColorU32(IM_COL32(255, 255, 255, 255));
    //     drawList->AddRectFilledMultiColor(p0, p1, col_a, col_b, col_b, col_a);
    //     ImGui::InvisibleButton("##gradient1", gradient_size);
    // }
    // {
    //     ImVec2 p0 = ImGui::GetCursorScreenPos();
    //     ImVec2 p1 = ImVec2(p0.x + gradient_size.x, p0.y + gradient_size.y);
    //     ImU32 col_a = ImGui::GetColorU32(IM_COL32(0, 255, 0, 255));
    //     ImU32 col_b = ImGui::GetColorU32(IM_COL32(255, 0, 0, 255));
    //     drawList->AddRectFilledMultiColor(p0, p1, col_a, col_b, col_b, col_a);
    //     ImGui::InvisibleButton("##gradient2", gradient_size);
    // }


    // auto HelpQuestionMark = [](const char* desc) {
    //     ImGui::TextDisabled("(?)");
    //     if (ImGui::BeginItemTooltip())
    //     {
    //         drawList->AddRectFilledMultiColor(const ImVec2 &p_min, const ImVec2 &p_max, ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left)
    //         ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
    //         ImGui::TextUnformatted(desc);
    //         ImGui::PopTextWrapPos();
    //         ImGui::EndTooltip();
    //     }
    // };




    u32 selected_source = DEFAULT32;
    bool need_to_add_source = false;
    const char* names[] = { 
        "Static  Force", 
        "Static  Dye", 
        "Static  Force+Dye", 
        "Dynamic Force",
        "Dynamic Dye",
        "Dynamic Force+Dye"
    };

    if (ImGui::Button("Add Source"))
        ImGui::OpenPopup("add_source_popup");

    if (ImGui::BeginPopup("add_source_popup"))
    {
        ImGui::SeparatorText("\
Static/Dynamic - Press <A>\n\
Static:  Change Position Using the Menu Only\n\
Dynamic: Change Position Using the Mouse\n"
        );
        for (u32 i = 0; i < __carraysize(names); ++i)
            if (ImGui::Selectable(names[i])) {
                selected_source = i;
                need_to_add_source = true;
            }
        ImGui::EndPopup();
    }


    if(need_to_add_source) {
        FluidSource __defsource{
            0, 1,
            FillType::FORCE,
            {0},
            0.01f,
            vec2f{0, 0},
            vec4f{0.0f},
            vec4f{ random32f(), random32f(), random32f(), 1.0f }
        };
        switch(selected_source) {
            case DEFAULT32:
            break;
            case 0:
            break;
            case 1:
            __defsource.m_type = FillType::DYE;
            break;
            case 2:
            __defsource.m_type = FillType::FORCE_AND_DYE;
            break;
            case 3:
            __defsource.m_static = false;
            break;
            case 4:
            __defsource.m_static = false;
            __defsource.m_type   = FillType::DYE;
            break;
            case 5:
            __defsource.m_static = false;
            __defsource.m_type   = FillType::FORCE_AND_DYE;
            break;
            default: /* should never reach */
            break;
        }
        g_sources.push_back(__defsource);
    }


    if (ImGui::TreeNodeEx("Fluid Sources", 
        ImGuiTreeNodeFlags_FramePadding
        |
        ImGuiTreeNodeFlags_DefaultOpen
    )) {
        for (u64 n = 0; n < g_sources.size(); n++)
        {
            auto& source = g_sources[n];


            static char label[32];
            snprintf(label, __carraysize(label), "%s %s %s", 
                source.m_static ? "STATIC " : "DYNAMIC",
                source.m_enabled ? "ENABLED " : "DISABLED",
                fillTypeToString(source.m_type)
            );
            ImGui::PushID(n);
            if (ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(60);
                if(ImGui::SmallButton("Enable/Disable")) {
                    source.m_enabled   = !source.m_enabled;
                    g_mostRecentSource = n;
                }
                // if(ImGui::SmallButton("Delete")) {
                //     g_mostRecentSource = n;
                // }
                ImGui::PopItemWidth();

                // ImGui::SameLine();
                
                // ImGui::PushItemWidth(60);
                // if(ImGui::Button("Force/Dye"))
                //     source.m_type = (source.m_type == FillType::DYE) 
                //     ? 
                //     FillType::FORCE : FillType::DYE;
                // ImGui::PopItemWidth();
                

                ImGui::Text("     Position      ");
                ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
                ImGui::Text("Radius   ");
                ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
                ImGui::Text("Value");

                
                ImGui::PushItemWidth(60);
                ImGui::PushID("DragFloatWinSizeX");
                ImGui::PushID(n);
                if(ImGui::DragFloat("", &source.m_position.x, 0.5f, 0.0f, __scast(f32, winsize.x))) {
                    g_mostRecentSource = n;
                }
                ImGui::PopID();
                ImGui::PopID();
                ImGui::PopItemWidth();
                ImGui::SameLine();
                ImGui::PushItemWidth(60);
                ImGui::PushID("DragFloatWinSizeY");
                ImGui::PushID(n);
                if(ImGui::DragFloat("", &source.m_position.y, 0.5f, 0.0f, __scast(f32, winsize.y))) {
                    g_mostRecentSource = n;
                }
                ImGui::PopID();
                ImGui::PopID();
                ImGui::PopItemWidth();
                if(!source.m_static && source.m_enabled) {
                    auto mousepos   = awc2getMousePosition();
                    auto mousedelta = awc2getMousePositionDelta();
                    source.m_position = { mousepos.x, mousepos.y };
                    source.m_force    = { mousedelta.x, mousedelta.y, 0.0f, 0.0f };
                }

                ImGui::SameLine();

                ImGui::PushItemWidth(60);
                ImGui::PushID("DragFloatSourceRadius");
                ImGui::PushID(n);
                if(ImGui::DragFloat("", &source.m_radius, 0.0001f, 0.0001f, 0.5f, "%-8.6f")) {
                    g_mostRecentSource = n;
                }
                ImGui::PopID();
                ImGui::PopID();
                ImGui::PopItemWidth();
                
                ImGui::SameLine();

                ImGui::PushID("SourceColorButton");
                ImGui::PushID(n);
                if(source.m_type == FillType::FORCE_AND_DYE) {
                    ImGui::PushID("ColorButtonForceInput");
                    if(ImGui::ColorButton("", *__rcast(ImVec4*, &source.m_force))) {
                        source.m_force = vec4f{ random32f(), random32f(), 0.0f, 1.0f };
                        g_mostRecentSource = n;
                    }
                    ImGui::PopID(); 
                    ImGui::SameLine();
                }
                if(ImGui::ColorButton("", *__rcast(ImVec4*, &source.m_color))) {
                    source.m_color = vec4f{ random32f(), random32f(), random32f(), 1.0f };
                    g_mostRecentSource = n;
                }
                ImGui::PopID(); 
                ImGui::PopID();


                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        ImGui::TreePop();
    }
    if(g_mostRecentSource != DEFAULT64 && awc2isKeyPressed(AWC2_KEYCODE_A)) {
        g_sources[g_mostRecentSource].m_static = !g_sources[g_mostRecentSource].m_static;
    }



    if(frameTimeNs < g_maxFrameTimeNs) {
        g_normdt = g_dt / __frameTime;
    }
    g_normdt = g_dt;
    g_cfl = g_normdt * (
        + g_maxVelocity.x / g_unitLength
        + g_maxVelocity.y / g_unitLength
    );
    

    vec2f tmp0 = g_maxVelocity * (g_unitLength / g_kinematicViscosity);
    g_reynolds = tmp0.x + tmp0.y;


    ImGui::SeparatorText("Diagnostics");
    ImGui::Text(imgui_diagnostics_text,
        awc2getCurrentContextViewport().x, awc2getCurrentContextViewport().y, g_dims.x, g_dims.y,

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

        g_frameCounter, __frameTime, 
        __scast(f32, g_minFrameTimeNs) * 1e-6, 
        __scast(f32, g_maxFrameTimeNs) * 1e-6,
        __scast(f32, g_avgFrameTimeNs) * 1e-6 / __scast(f32, g_frameCounter),

        __beginFrameTime,
        __fluidUpdateTime,
        __computeVelTime,
        __computeDyeTime,
        __computeCFLTime,
        __computeMaximumGPU,
        __computeMaximumCPU,
        __computeErrEstimateTime,
        __computeErrorGPU,
        __computeErrorCPU,
        __renderScreenTime,
        __renderImGuiTime,
        __renderBlitTime,
        __endFrameTime,
        g_enableGPUTimers ? "ENABLED " : "DISABLED",
        __scast(f64, g_computeVelTimeGPU.previousFrame()) * 1e-6,
        __scast(f64, g_computeDyeTimeGPU.previousFrame()) * 1e-6,
        __scast(f64, g_computeCFLTimeGPU.previousFrame()) * 1e-6,
        __scast(f64, g_computeErrTimeGPU.previousFrame()) * 1e-6,
        __scast(f64, g_computeScreenTimeGPU.previousFrame()) * 1e-6
    );
    ImGui::End();

    return;
}