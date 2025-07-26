#include "advection_boundary.hpp"
#include "gl/shader2.hpp"
#include <awc2/C/awc2.h>
#include <glbinding/gl/gl.h>
#include <threads.h>
#include <util2/C/marker4.h>
#include <util2/random.hpp>
#include <util2/vec2.hpp>


using namespace util2::math;


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
            // field[i + j * dims.x] = vec4f{ 
            //     __scast(f32, i) * dimsf.x,
            //     __scast(f32, j) * dimsf.y,
            //     0.0f, 1.0f
            // };
            field[i + j * dims.x] = vec4f{ 
                util2::random32f(),
                util2::random32f(),
                0.0f,
                1.0f
            };
        }
    }
    mark();
    return;
}



static constexpr u8       kg_slowRender{false};
static std::vector<vec4f> g_initialField;
static u64                g_frameCounter{0};
static vec2u              g_dims{1920, 1080};
static f32                g_dt = 0.01f;
static u32                g_texture[3];
static u32                g_fbo;
static ShaderProgramV2    g_compute;




static inline void render()
{
    static const vec4f defaultScreenColor = vec4f{1.0f, 1.0f, 0.1f, 1.0f}; 
    auto currentWindowSize = awc2getCurrentContextViewport();

    u8 status = 1;
    if(awc2isKeyPressed(AWC2_KEYCODE_R)) {
        g_compute.refreshFromFiles();
        g_compute.resizeLocalWorkGroup(0, 1, 1, 1);
        status = g_compute.compile();
    }
    if(!status)
        return; /* couldn't recompile successfully, code needs recheck */



    gl::glClearNamedFramebufferfv( /* Clear Screen */
        g_fbo, 
        gl::GL_COLOR, 
        0, 
        defaultScreenColor.begin()
    );


    /* bind compute shader & bind texture, also fbo */
    vec2i cvt = vec2i{g_dims.x, g_dims.y};
    g_compute.bind();
    g_compute.uniform1i("renderto",    1);
    g_compute.uniform1i("sim_step_u0", 2);
    g_compute.uniform1i("sim_step_u1", 3);
    g_compute.uniform1f("ku_dt", g_dt);
    g_compute.uniform2iv("ku_simdims", cvt.begin());
    gl::glBindImageTexture(1, g_texture[0], 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    gl::glBindTextureUnit(2, g_texture[1]);
    gl::glBindImageTexture(3, g_texture[2], 0, false, 0, 
        gl::GL_READ_WRITE, 
        gl::GL_RGBA32F
    );

    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
    gl::glDispatchCompute(g_dims.x, g_dims.y, 1);


    /* Draw Call */
    gl::glBlitNamedFramebuffer(g_fbo, 0, 
        0, 0, g_dims.x,     g_dims.y, 
        0, 0, currentWindowSize.x, currentWindowSize.y,
        gl::GL_COLOR_BUFFER_BIT, 
        gl::GL_LINEAR
    );
    
    u32 tmp = g_texture[2]; 
    g_texture[2] = g_texture[1];
    g_texture[1] = tmp;
    ++g_frameCounter;
}


i32 advection_and_boundary_conditions()
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

#if defined __linux__
    static constexpr const char* computeShaderFilename = "projects/program/source/4advection_boundary/advection_boundary.comp";
#elif defined _WIN32
    static constexpr const char* computeShaderFilename = "C:/CTools/Projects/main/projects/program/source/4advection_boundary/advection_boundary.comp";
#endif


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


    markstr("Graphics Init Begin"); /* init graphics data */
    gl::glCreateTextures(gl::GL_TEXTURE_2D, 3, &g_texture[0]);
    gl::glCreateFramebuffers(1, &g_fbo);
    g_compute.createFrom({
        ShaderData{ computeShaderFilename, __scast(u32, gl::GL_COMPUTE_SHADER) }
    });
    g_compute.resizeLocalWorkGroup(0, 1, 1, 1);


    /* setup draw texture */
    for(auto& tex : g_texture) {
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_WRAP_S, gl::GL_CLAMP_TO_EDGE);
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_WRAP_T, gl::GL_CLAMP_TO_EDGE);
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_MIN_FILTER, gl::GL_LINEAR);
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_MAG_FILTER, gl::GL_LINEAR);
        gl::glTextureStorage2D(tex, 1, gl::GL_RGBA32F, g_dims.x, g_dims.y);
    }
    
    initializeVectorFields(g_dims, g_initialField);
    gl::glTextureSubImage2D(g_texture[1], 0, 0, 0, 
        g_dims.x, g_dims.y,
        gl::GL_RGBA, 
        gl::GL_FLOAT,
        g_initialField.data()
    );


    gl::glNamedFramebufferTexture(g_fbo, gl::GL_COLOR_ATTACHMENT0, g_texture[0], 0);
    ifcrash( gl::glCheckNamedFramebufferStatus(g_fbo, gl::GL_FRAMEBUFFER) 
        != gl::GL_FRAMEBUFFER_COMPLETE 
    );
    ifcrashstr(!g_compute.compile(), "Unsuccessful shader compile");


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
    gl::glDeleteTextures(3, &g_texture[0]);
    gl::glDeleteFramebuffers(1, &g_fbo);
    g_compute.destroy();
    g_initialField.resize(0);
    markstr("Graphics Destroy End");

    markstr("AWC2 Destroy Begin");
    awc2destroyContext(contextid);
    awc2destroy();
    markstr("AWC2 Destroy End");



    markstr("gpu_gems38 end  ");
    return 1;
}