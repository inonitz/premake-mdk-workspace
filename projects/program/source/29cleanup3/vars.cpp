#include "vars.hpp"
#include "util/time.hpp"
#include <glbinding/gl/gl.h>
#include <util/aligned_malloc.hpp>
#include <util/marker2.hpp>
#include <awc2/C/awc2.h>


namespace cleanup329 {


const char* computeShaderFilename[10] = {
    "projects/program/source/29cleanup3/shaders/0force.comp",
    "projects/program/source/29cleanup3/shaders/1advect.comp",
    "projects/program/source/29cleanup3/shaders/2diffusev.comp",
    "projects/program/source/29cleanup3/shaders/3div.comp",
    "projects/program/source/29cleanup3/shaders/4diffusep.comp",
    "projects/program/source/29cleanup3/shaders/5final.comp",
    "projects/program/source/29cleanup3/shaders/6draw.comp",
    "projects/program/source/29cleanup3/shaders/minmax.comp",
    "projects/program/source/29cleanup3/shaders/errortex.comp",
    "projects/program/source/29cleanup3/shaders/error_reduct.comp",
};


/* 
    Time Measurements / Constants
*/
u8                 g_contextid;
u32                g_frameCounter{0};
i64                g_minFrameTimeNs{1'000'000'000ll};
i64                g_maxFrameTimeNs{0};
i64                g_avgFrameTimeNs{0};
const i64          g_slowRenderDurationNs{50 * 1'000'000'000ll};
i64                g_waitTime{};
Time::Timestamp    g_timerBuffer[16]{};
Time::Timestamp&   g_frameTime              = g_timerBuffer[0];
Time::Timestamp&   g_renderTime             = g_timerBuffer[1];
Time::Timestamp&   g_beginFrameTime         = g_timerBuffer[2];
Time::Timestamp&   g_endFrameTime           = g_timerBuffer[3];
Time::Timestamp&   g_computeFluidTime       = g_timerBuffer[4];
Time::Timestamp&   g_computeVelTime         = g_timerBuffer[5];
Time::Timestamp&   g_computeDyeTime         = g_timerBuffer[6];
Time::Timestamp&   g_computeCFLTime         = g_timerBuffer[7];
Time::Timestamp&   g_computeErrEstimateTime = g_timerBuffer[8];
Time::Timestamp&   g_computeMaximumCPU      = g_timerBuffer[9];
Time::Timestamp&   g_computeMaximumGPU      = g_timerBuffer[10];
Time::Timestamp&   g_computeErrorGPU        = g_timerBuffer[11];
Time::Timestamp&   g_computeErrorCPU        = g_timerBuffer[12];
Time::Timestamp&   g_renderImguiTime        = g_timerBuffer[13];
Time::Timestamp&   g_renderScreenTime       = g_timerBuffer[14];


/* Compute Parameters */
const vec2i        g_dims{1024, 1024};
const vec2f        g_invdims = vec2f{
    1.0f / __scast(f32, g_dims.x),
    1.0f / __scast(f32, g_dims.y),
};
const vec3u        g_localWorkGroupSize = { 64, 1, 1 };
const vec3u        g_computeInvocationSize = vec3u{ g_dims.x, g_dims.y, 1 } / g_localWorkGroupSize;
i32                g_maximumJacobiIterations = 80;
const i32          g_reductionFactor       = 8192;
const i32          g_reductionBufferLength = g_dims.x * g_dims.y / g_reductionFactor;


/* Simulation Constants */
f32   g_kinematicViscosity = 1.0f;
f32   g_densityPressure    = 1.0f;
f32   g_confineVorticity   = 0.0f;
f32   g_buoyancyStrength   = 1.0f;
f32   g_ambientTemperature = 20.0f;
f32   g_dt                 = 0.08f;
f32   g_normdt             = g_dt;
f32   g_cfl                = 0.0f;
f32   g_reynolds           = 0.0f;
f32   g_unitLength         = 1.0f;


/* User Interaction/Info */
std::vector<FluidSource> g_sources{};
bool       g_imGuiButton[20]{false};
DrawTarget g_chooseTextureToRender{DrawTarget::VELOCITY_PRESSURE};
f32        g_textureHighlightSmallValue{100.0f};
f32        g_maxSpectralRadius{0.0f};
f32        g_iterationErrorN{0.0f};
vec2f      g_maxVelocity{0.0f};
vec4f g_prevErrorValues[3]{ /* min, max, avg */
    vec4f{1000.0f},
    vec4f{0.0f},
    vec4f{0.0f}
};
vec4f g_currErrorValues[3]{ /* min, max, avg */
    vec4f{1000.0f},
    vec4f{0.0f},
    vec4f{0.0f}
};


/* OpenGL Data */
gl::GLsync         g_fence; 
u32                g_texture[19];
u32                g_persistentbuf[2];
void*              g_mappedBuffer[2];
u32                g_fbo;
ShaderProgramV2    g_compute[__carraysize(computeShaderFilename)];



ShaderProgramV2& gr_computeInteractive       = g_compute[0];
ShaderProgramV2& gr_computeAdvection         = g_compute[1];
ShaderProgramV2& gr_computeDiffusionVel      = g_compute[2];
ShaderProgramV2& gr_computeDivergence        = g_compute[3];
ShaderProgramV2& gr_computeDiffusionPressure = g_compute[4];
ShaderProgramV2& gr_computeNewVelocity       = g_compute[5];
ShaderProgramV2& gr_computeRenderToTex       = g_compute[6];
ShaderProgramV2& gr_computeCFLCondition      = g_compute[7];
ShaderProgramV2& gr_computeErrorTexture      = g_compute[8];
ShaderProgramV2& gr_computeErrorEstimates    = g_compute[9];


const u32& gr_drawTexture   = g_texture[0];
const u32& gr_dyeTexture0   = g_texture[1];
const u32& gr_dyeTexture1   = g_texture[2];
const u32& gr_tmpTexture0   = g_texture[3];
const u32& gr_tmpTexture1   = g_texture[4];
const u32& gr_outTexShader0 = g_texture[5];
const u32& gr_outTexShader1 = g_texture[6];
const u32& gr_outTexShader2 = g_texture[7]; /* will use tmpTexture for ping-pong */
const u32& gr_outTexShader3 = g_texture[8];
const u32& gr_outTexShader4 = g_texture[9];
const u32& gr_outTexShader5 = g_texture[10];
const u32& gr_outTexShader6 = g_texture[11];
const u32& gr_outTexShader7 = g_texture[12];
const u32& gr_outTexShader8 = g_texture[13];
const u32& gr_outTexShader9 = g_texture[14];
const u32& gr_simTexture0   = g_texture[15]; /* Components are [u.x, u.y, p, reserved] */
const u32& gr_simTexture1   = g_texture[16];
const u32& gr_errorTexture  = gr_outTexShader9;
const u32& g_reductionErrBuffer = g_persistentbuf[0];
const u32& g_reductionMaxBuffer = g_persistentbuf[1];


u32 gr_prevIterVel = DEFAULT32;
u32 gr_nextIterVel = DEFAULT32;
u32 gr_prevIterDye = DEFAULT32;
u32 gr_nextIterDye = DEFAULT32;


void* g_reductionMaxMappedBuf = nullptr;
void* g_reductionErrMappedBuf = nullptr;




void init::initializeLibrary()
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
    awc2WindowDescriptorDefault(&ctxtinfo.winDesc);
    awc2initializeContext(&ctxtinfo);
    markstr("AWC2 init end");
    return;
}


void init::destroyLibrary()
{
    markstr("AWC2 Destroy Begin");
    awc2destroyContext(g_contextid);
    awc2destroy();
    markstr("AWC2 Destroy End");
    return;
}




void init::initializeGraphics()
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
        g_compute[i].resizeLocalWorkGroup(0,
            g_localWorkGroupSize.x,
            g_localWorkGroupSize.y,
            g_localWorkGroupSize.z
        );
        alive = alive && g_compute[i].compile();
    }
    ifcrashstr(!alive, "Unsuccessful shader compile");


    gr_prevIterVel = gr_simTexture0;
    gr_nextIterVel = gr_simTexture1;
    gr_prevIterDye = gr_dyeTexture0;
    gr_nextIterDye = gr_dyeTexture1;
    for(u32 i = 0; i < __carraysize(g_texture); ++i) {
        auto& tex = g_texture[i];
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_WRAP_S, gl::GL_CLAMP_TO_EDGE);
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_WRAP_T, gl::GL_CLAMP_TO_EDGE);
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_MIN_FILTER, gl::GL_NEAREST);
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_MAG_FILTER, gl::GL_NEAREST);
        gl::glTextureStorage2D(tex, 1, gl::GL_RGBA32F, g_dims.x, g_dims.y);
        gl::glClearTexImage(tex, 0, gl::GL_RGBA, gl::GL_FLOAT, nullptr);
    }


    gl::glNamedBufferStorage(g_reductionErrBuffer, 
        g_reductionBufferLength * 3 * sizeof(vec4f), 
        nullptr, 
        gl::GL_NONE_BIT
        | gl::GL_MAP_READ_BIT
        | gl::GL_MAP_PERSISTENT_BIT
        | gl::GL_MAP_COHERENT_BIT
    );
    g_reductionErrMappedBuf = gl::glMapNamedBufferRange(g_reductionErrBuffer, 
        0, g_reductionBufferLength * 3 * sizeof(vec4f),  
        gl::GL_NONE_BIT
        | gl::GL_MAP_READ_BIT
        | gl::GL_MAP_PERSISTENT_BIT
        | gl::GL_MAP_COHERENT_BIT
    );

    gl::glNamedBufferStorage(g_reductionMaxBuffer, 
        g_reductionBufferLength * sizeof(vec4f), 
        nullptr, 
        gl::GL_NONE_BIT
        | gl::GL_MAP_READ_BIT
        | gl::GL_MAP_PERSISTENT_BIT
        | gl::GL_MAP_COHERENT_BIT
    );
    g_reductionMaxMappedBuf = gl::glMapNamedBufferRange(g_reductionMaxBuffer, 
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


void init::destroyGraphics()
{
    markstr("Graphics Destroy Begin");


    gl::glDeleteTextures(__carraysize(g_texture), &g_texture[0]);
    gl::glUnmapNamedBuffer(g_reductionMaxBuffer);
    gl::glUnmapNamedBuffer(g_reductionErrBuffer);
    gl::glDeleteBuffers(__carraysize(g_persistentbuf), &g_persistentbuf[0]);
    gl::glDeleteFramebuffers(1, &g_fbo);
    for(auto& comp : g_compute) {
        comp.destroy();
    }


    markstr("Graphics Destroy End");
    return;
}


} /* namespace cleanup329 */
