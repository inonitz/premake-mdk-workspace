#include <util2/C/macro.h>
#include <util2/C/base_type.h>
#include <util2/C/marker4.h>


#ifndef _SELECT_MAIN
#   define _SELECT_MAIN 29
#endif

#if _SELECT_MAIN == -1
#   include "imgui_demo/render_demo.hpp"
#elif _SELECT_MAIN == 0
#   include "0gem38/gem38.hpp"
#elif _SELECT_MAIN == 1
#   include "1simple_window/simple_window.hpp"
#elif _SELECT_MAIN == 2
#   include "2compute_screen/compute_screen.hpp"
#elif _SELECT_MAIN == 3
#   include "3compute_buffer/compute_buffer.hpp"
#elif _SELECT_MAIN == 4
#   include "4advection_boundary/advection_boundary.hpp"
#elif _SELECT_MAIN == 5
#   include "5test_diffusion/test_diffusion.hpp"
#elif _SELECT_MAIN == 6
#   include "6compute_buffer_mouse/compute_buffer_mouse.hpp"
#elif _SELECT_MAIN == 7
#   include "7compute_buffer_mouse_diffusion/compute_buffer_mouse_diffusion.hpp"
#elif _SELECT_MAIN == 8
#   include "8compute_buffer_mouse_advection_diffusion/compute_buffer_mouse_advection_diffusion.hpp"
#elif _SELECT_MAIN == 9
#   include "9forces_advection_diffusion/forces_advection_diffusion.hpp"
#elif _SELECT_MAIN == 10
#   include "10velocityfield/velocityfield.hpp"
#elif _SELECT_MAIN == 11
#   include "11fullsim/fullsim.hpp"
#elif _SELECT_MAIN == 12
#   include "12cfl/cfl.hpp"
#elif _SELECT_MAIN == 13
#   include "13dye/dye.hpp"
#elif _SELECT_MAIN == 14
#   include "14measure/measure.hpp"
#elif _SELECT_MAIN == 15
#   include "15optimize/optimize.hpp"
#elif _SELECT_MAIN == 16
#   include "16makessbowork/ssbo.hpp"
#elif _SELECT_MAIN == 17
#   include "17optimize2/optimize2.hpp"
#elif _SELECT_MAIN == 18
#   include "18features/features.hpp"
#elif _SELECT_MAIN == 19
#   include "19cleanup/cleanup.hpp"
#elif _SELECT_MAIN == 20
#   include "20smoke/smoke.hpp"
#elif _SELECT_MAIN == 21
#   include "21boundaries/arbitrary.hpp"
#elif _SELECT_MAIN == 23
#   include "23multigrid/mg.hpp"
#elif _SELECT_MAIN == 24
#   include "24morebutton/mb.hpp"
#elif _SELECT_MAIN == 25
#   include "25boundaryagain/ba.hpp"
#elif _SELECT_MAIN == 26
#   include "26anotherattempt/aa.hpp"
#elif _SELECT_MAIN == 27
#   include "27testsubimage/subimage.hpp"
#elif _SELECT_MAIN == 28
#   include "28cleanup2/demo.hpp"
#elif _SELECT_MAIN == 29
#   include "29cleanup3/improve.hpp"
#endif




int main(__unused int argc, __unused char* argv[]) {
    i32 out = 0x69;
    markstr("Successful main enter");

#if _SELECT_MAIN == -1
    out = render_imgui_demo_window();
#elif _SELECT_MAIN == 0
    out = gpugems38_main();
#elif _SELECT_MAIN == 1
    out = simple_window();
#elif _SELECT_MAIN == 2
    out = compute_shader_render_to_screen();
#elif _SELECT_MAIN == 3
    out = compute_shader_render_buffer_to_screen();
#elif _SELECT_MAIN == 4
    out = advection_and_boundary_conditions();
#elif _SELECT_MAIN == 5
    out = test_diffusion();
#elif _SELECT_MAIN == 6
    out = compute_shader_render_buffer_to_screen_mouse_interaction();
#elif _SELECT_MAIN == 7
    out = compute_buffer_mouse_but_with_diffusion();
#elif _SELECT_MAIN == 8
    out = compute_buffer_mouse_but_with_advection_and_diffusion();
#elif _SELECT_MAIN == 9
    out = compute_shader_external_forces_advection_diffusion_draw();
#elif _SELECT_MAIN == 10
    out = compute_shader_draw_full_velocity_field();
#elif _SELECT_MAIN == 11
    out = compute_shader_draw_full_simulation_field();
#elif _SELECT_MAIN == 12
    out = compute_shader_2d_fluid_also_cfl();
#elif _SELECT_MAIN == 13
    out = fluid_sim_2d_also_dye();
#elif _SELECT_MAIN == 14
    out = started_optimization_derailed_to_measuring_everything();
#elif _SELECT_MAIN == 15
    out = after_measuring_you_cut_the_dead_wood();
#elif _SELECT_MAIN == 16
    out = make_ssbo_work_for_cfl();
#elif _SELECT_MAIN == 17
    out = optimize_again_and_again_and_again_and_again();
#elif _SELECT_MAIN == 18
    out = add_features_and_then_optimize();
#elif _SELECT_MAIN == 19
    out = cleanup_old();
#elif _SELECT_MAIN == 20
    out = smoke_sim();
#elif _SELECT_MAIN == 21
    out = arbitrary_boundaries_in_sim();
#elif _SELECT_MAIN == 23
    out = multigrid_method_also_no_internal_boundaries_for_now();
#elif _SELECT_MAIN == 24
    out = no_multigrid_for_now_more_buttons();
#elif _SELECT_MAIN == 25
    out = no_multigrid_for_now_until_internal_boundary_works();
#elif _SELECT_MAIN == 26
    out = another_boundary_implementation_attempt();
#elif _SELECT_MAIN == 27
    out = compute_user_interaction_with_subimage();
#elif _SELECT_MAIN == 28
    out = gpugems38_demo();
#elif _SELECT_MAIN == 29
        out = cleanup329::gpugems38_demo_last();
#endif


    markstr("Successful Exit");
    return out;
}