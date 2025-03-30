#include "test_diffusion.hpp"
#include <threads.h>
#include <glbinding/gl/gl.h>
#include <util/marker2.hpp>
#include <util/vec2.hpp>
#include <util/random.hpp>
#include <awc2/C/awc2.h>
#include <util/marker2.hpp>
#include "gl/shader2.hpp"
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


static inline void initializeVectorFields(vec2u const& dims, std::vector<vec4f>& field) {
    mark();
    field.resize(dims.x * dims.y);
    vec2f dimsf = vec2f{ dims.x, dims.y };

    dimsf = vec2f{1.0f} / dimsf; 
    for(u32 j = 0; j < dims.y; ++j) {
        for(u32 i = 0; i < dims.x; ++i) {
            field[i + j * dims.x] = vec4f{0.0f};
        }
    }
    mark();
    return;
}


static void initializeExternForces(vec2u const& dims, std::vector<vec4f>& field) {
    mark();
    field.resize(dims.x * dims.y);
    vec2f dimsf = vec2f{ dims.x, dims.y };

    dimsf = vec2f{1.0f} / dimsf; 
    for(u32 j = 0; j < dims.y; ++j) {
        for(u32 i = 0; i < dims.x; ++i) {
            // field[i + j * dims.x] = vec4f{ 
            //     __scast(f32, i) * dimsf.x,
            //     __scast(f32, j) * dimsf.y,
            //     0.0f, 1.0f
            // };
            // field[i + j * dims.x] = vec4f{dimsf.x, dimsf.y, 0.0f, 0.0f}; /* simulating gravity for a sec */
            if(random32f() > 0.7f)
                field[i + j * dims.x] = vec4f{ random32f() * dimsf.x, random32f() * dimsf.y, 0.0f, 0.0f };
        }
    }
    mark();
    return;
}




#if defined __linux__
    static constexpr const char* gk_computeShaderFilename[4] = {
        "projects/program/source/5test_diffusion/0boundary.comp",
        "projects/program/source/5test_diffusion/1advect_extern_force.comp",
        "projects/program/source/5test_diffusion/2diffusion.comp",
        "projects/program/source/5test_diffusion/3draw.comp"
    };
#elif defined _WIN32
    static constexpr const char* gk_computeShaderFilename[4] = {
        "C:/CTools/Projects/main/projects/program/source/5test_diffusion/0boundary.comp",
        "C:/CTools/Projects/main/projects/program/source/5test_diffusion/1advect_extern_force.comp",
        "C:/CTools/Projects/main/projects/program/source/5test_diffusion/2diffusion.comp",
        "C:/CTools/Projects/main/projects/program/source/5test_diffusion/3draw.comp"
    };
#endif


static constexpr u8       kg_slowRender{false};
static constexpr u32      kg_jacobiSolverIterations = 60;
static std::vector<vec4f> g_initialField;
static std::vector<vec4f> g_externalForces;
static u64                g_frameCounter{0};
static vec2u              g_dims{1920, 1080};
static f32                g_dt        = 0.01f;
static f32                g_viscosity = 0.2f;
static u32                g_texture[10];
static u32                g_fbo;
static ShaderProgramV2    g_compute[__carraysize(gk_computeShaderFilename)];


static ShaderProgramV2& gr_compute0 = g_compute[0];
static ShaderProgramV2& gr_compute1 = g_compute[1];
static ShaderProgramV2& gr_compute2 = g_compute[2];
static ShaderProgramV2& gr_compute3 = g_compute[3];
static u32&             gr_vel_pressure = g_texture[0];
static u32&             gr_extern_force = g_texture[1];
static u32&             gr_diffusion    = g_texture[2];
static u32&             gr_scratch_tex0 = g_texture[5];
static u32&             gr_scratch_tex1 = g_texture[6];
static u32&             gr_scratch_tex2 = g_texture[7];
static u32&             gr_drawTexture  = g_texture[9];


static void initializeTextures();
static void destroyTextures();
static void simulationSingleIteration();
static void renderToDrawTexture();
static void render();


i32 test_diffusion()
{
    const struct timespec pause_sleep_duration{
        .tv_sec = 0,
        .tv_nsec = 6944444
    };
    const struct timespec slow_render_sleep_duration{
        .tv_sec = 0,
        .tv_nsec = 300 * 1000000
    };
    u8 alive {true};
    u8 paused{false};
    u8 contextid;


    markstr("gpu_gems38 begin");
    
    
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


    markstr("Graphics Init Begin");
    bool ok = true;
    for(u8 i = 0; ok && i < __carraysize(gk_computeShaderFilename); ++i) {
        g_compute[i].createFrom({
            ShaderData{ gk_computeShaderFilename[i], __scast(u32, gl::GL_COMPUTE_SHADER) }
        });
        g_compute[i].resizeLocalWorkGroup(0, 1, 1, 1);
        ok = ok && g_compute[i].compile();
    }
    ifcrashstr(!ok, "Unsuccessful shader compile");

    initializeTextures();
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
            if constexpr (kg_slowRender) {
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
    destroyTextures();
    for(auto& shader : g_compute) {
        shader.destroy();
    }
    g_initialField.resize(0);
    markstr("Graphics Destroy End");

    markstr("AWC2 Destroy Begin");
    awc2destroyContext(contextid);
    awc2destroy();
    markstr("AWC2 Destroy End");



    markstr("gpu_gems38 end  ");
    return 1;
}




static void initializeTextures()
{
    mark();
    mark(); gl::glCreateTextures(gl::GL_TEXTURE_2D, __carraysize(g_texture), &g_texture[0]);
    mark(); gl::glCreateFramebuffers(1, &g_fbo);


    /* setup draw texture */
    mark(); for(auto& tex : g_texture) {
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_WRAP_S, gl::GL_CLAMP_TO_EDGE);
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_WRAP_T, gl::GL_CLAMP_TO_EDGE);
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_MIN_FILTER, gl::GL_LINEAR);
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_MAG_FILTER, gl::GL_LINEAR);
        gl::glTextureStorage2D(tex, 1, gl::GL_RGBA32F, g_dims.x, g_dims.y);
    }
    
    mark(); initializeVectorFields(g_dims, g_initialField);
    mark(); initializeExternForces(g_dims, g_externalForces);
    mark(); gl::glTextureSubImage2D(gr_vel_pressure, 0, 0, 0, 
        g_dims.x, g_dims.y,
        gl::GL_RGBA, 
        gl::GL_FLOAT,
        g_initialField.data()
    );
    mark(); gl::glTextureSubImage2D(gr_extern_force, 0, 0, 0, 
        g_dims.x, g_dims.y,
        gl::GL_RGBA, 
        gl::GL_FLOAT,
        g_externalForces.data()
    );


    mark(); gl::glNamedFramebufferTexture(g_fbo, gl::GL_COLOR_ATTACHMENT0, gr_drawTexture, 0);
    ifcrash( gl::glCheckNamedFramebufferStatus(g_fbo, gl::GL_FRAMEBUFFER) 
        != gl::GL_FRAMEBUFFER_COMPLETE 
    );
    mark();
    return;
}


static void destroyTextures()
{
    gl::glDeleteTextures(__carraysize(g_texture), &g_texture[0]);
    gl::glDeleteFramebuffers(1, &g_fbo);
    return;
}


static void render()
{
    static const vec4f defaultScreenColor = vec4f{1.0f, 1.0f, 0.1f, 1.0f}; 
    auto currentWindowSize = awc2getCurrentContextViewport();

    u8 status = 1;
    if(awc2isKeyPressed(AWC2_KEYCODE_R)) {
        // g_compute.refreshFromFiles();
        // g_compute.resizeLocalWorkGroup(0, 1, 1, 1 );
        // status = g_compute.compile();
    }
    if(!status)
        return; /* couldn't recompile successfully, code needs recheck */



    gl::glClearNamedFramebufferfv( /* Clear Screen */
        g_fbo, 
        gl::GL_COLOR, 
        0, 
        defaultScreenColor.begin()
    );
    simulationSingleIteration();
    renderToDrawTexture();


    gl::glBlitNamedFramebuffer(g_fbo, 0, /* Draw Call */
        0, 0, g_dims.x,     g_dims.y, 
        0, 0, currentWindowSize.x, currentWindowSize.y,
        gl::GL_COLOR_BUFFER_BIT, 
        gl::GL_LINEAR
    );
    return;
}


static void renderToDrawTexture()
{


    
    vec2i cvt = vec2i{g_dims.x, g_dims.y};
    gr_compute3.bind();
    gr_compute3.uniform1i("field_velocity_pressure", 0);
    gr_compute3.uniform1i("screen_texture", 1);
    gr_compute3.uniform2iv("ku_simdims", cvt.begin());
    gl::glBindTextureUnit(0, gr_diffusion);
    gl::glBindImageTexture(1, gr_drawTexture, 0, false, 0, gl::GL_WRITE_ONLY, gl::GL_RGBA32F);
    gl::glDispatchCompute(g_dims.x, g_dims.y, 1);
    gl::glMemoryBarrier(gl::MemoryBarrierMask::GL_ALL_BARRIER_BITS);
    return;
}


static void simulationSingleIteration()
{
    vec2i cvt = vec2i{g_dims.x, g_dims.y};
    u32 pingping_texid[2] = { 0, 0 };
    u32 jacobiCounter = kg_jacobiSolverIterations;
    // u32 jacobiCounter = 40;
    gl::MemoryBarrierMask shaderpassMask = 
        gl::MemoryBarrierMask::GL_UNIFORM_BARRIER_BIT |
        gl::MemoryBarrierMask::GL_TEXTURE_FETCH_BARRIER_BIT |
        gl::MemoryBarrierMask::GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;


    /* Shader step 0: apply boundary condition to existing velocity-pressure field */
    /* result written in gr_scratch_tex2 */
    gr_compute0.bind();
    gr_compute0.uniform1i("sim_velocity_pressure", 0);
    gr_compute0.uniform1i("sim_output", 1);
    gr_compute0.uniform2iv("ku_simdims", cvt.begin());
    gl::glBindTextureUnit(0, gr_vel_pressure);
    gl::glBindImageTexture(1, gr_scratch_tex2, 0, false, 0, gl::GL_WRITE_ONLY, gl::GL_RGBA32F);
    gl::glDispatchCompute(g_dims.x, g_dims.y, 1);
    gl::glMemoryBarrier(shaderpassMask);


    /* Shader step 1: compute advection and external forces */
    /* result written in gr_scratch_tex0 */
    gr_compute1.bind();
    gr_compute1.uniform1i("sim_velocity_pressure", 0);
    gr_compute1.uniform1i("sim_forces", 1);
    gr_compute1.uniform1i("sim_output", 2);
    gr_compute1.uniform1f("ku_dt", g_dt);
    gr_compute1.uniform1f("ku_viscosity", g_viscosity);
    gr_compute1.uniform2iv("ku_simdims", cvt.begin());
    gl::glBindTextureUnit(0, gr_scratch_tex2);
    gl::glBindTextureUnit(1, gr_extern_force);
    gl::glBindImageTexture(2, gr_scratch_tex0, 0, false, 0, gl::GL_WRITE_ONLY, gl::GL_RGBA32F);
    gl::glDispatchCompute(g_dims.x, g_dims.y, 1);


    /* shader step 2: compute diffusion. requires ping-pong between buffers until desired output is produced */
    /* result written finally in gr_diffusion */
    gr_compute2.bind();
    gr_compute2.uniform1i("sim_velocity_pressure", 0);
    gr_compute2.uniform1i("sim_output", 1);
    gr_compute2.uniform1f("ku_dt", g_dt);
    gr_compute2.uniform1f("ku_viscosity", g_viscosity);
    gr_compute2.uniform2iv("ku_simdims", cvt.begin());
    
    gl::glBindTextureUnit(0, gr_scratch_tex2);
    gl::glBindImageTexture(1, gr_scratch_tex1, 0, false, 0, gl::GL_WRITE_ONLY, gl::GL_RGBA32F);
    gl::glDispatchCompute(g_dims.x, g_dims.y, 1);
    gl::glMemoryBarrier(shaderpassMask);
    pingping_texid[0] = gr_scratch_tex1;
    pingping_texid[1] = gr_diffusion;
    while(jacobiCounter > 1) {
        gl::glBindTextureUnit(0, pingping_texid[0]);
        gl::glBindImageTexture(1, pingping_texid[1], 0, false, 0, gl::GL_WRITE_ONLY, gl::GL_RGBA32F);
        gl::glDispatchCompute(g_dims.x, g_dims.y, 1);
        gl::glMemoryBarrier(shaderpassMask);

        std::swap(pingping_texid[0], pingping_texid[1]);
        --jacobiCounter;
    }


    ++g_frameCounter;
    return;
}