#include "vars.hpp"
#include "gl/shader2.hpp"
#include "glbinding/gl/functions.h"
#include "util/aligned_malloc.hpp"
#include <awc2/C/awc2.h>
#include <util/marker2.hpp>


namespace makebelievebound25 {


const char* computeShaderFilename[10] = {
    "projects/program/source/25boundaryagain/shaders/0force.comp",
    "projects/program/source/25boundaryagain/shaders/1advect.comp",
    "projects/program/source/25boundaryagain/shaders/2diffuse.comp",
    "projects/program/source/25boundaryagain/shaders/3div.comp",
    "projects/program/source/25boundaryagain/shaders/4pressure.comp",
    "projects/program/source/25boundaryagain/shaders/5final.comp",
    "projects/program/source/25boundaryagain/shaders/6draw.comp",
    "projects/program/source/25boundaryagain/shaders/minmax.comp",
    "projects/program/source/25boundaryagain/shaders/errortex.comp",
    "projects/program/source/25boundaryagain/shaders/error_reduct.comp",
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
Time::Timestamp    g_frameTime{};
Time::Timestamp    g_renderTime{};
Time::Timestamp    g_beginFrameTime{};
Time::Timestamp    g_endFrameTime{};
Time::Timestamp    g_computeFluidTime{};
Time::Timestamp    g_computeVelTime{};
Time::Timestamp    g_computeDyeTime{};
Time::Timestamp    g_computeCFLTime{};
Time::Timestamp    g_computeErrEstimateTime{};
Time::Timestamp    g_computeMaximumCPU{};
Time::Timestamp    g_computeMaximumGPU{};
Time::Timestamp    g_computeErrorGPU{};
Time::Timestamp    g_computeErrorCPU{};
Time::Timestamp    g_renderImguiTime{};
Time::Timestamp    g_renderScreenTime{};


/* Compute Parameters */
const vec2i        g_dims{512, 512};
const vec2f        g_invdims = vec2f{
    1.0f / __scast(f32, g_dims.x),
    1.0f / __scast(f32, g_dims.y),
};
vec2i              g_windowSize{1920, 1080};
const vec3u        g_localWorkGroupSize = { 64, 1, 1 };
const vec3u        g_computeInvocationSize = vec3u{ g_dims.x, g_dims.y, 1 } / g_localWorkGroupSize;
i32                g_maximumJacobiIterations = 80;
const i32          g_reductionFactor = 8192;
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
vec2f g_simUnitCoords{1.0f};


/* User Interaction/Info */
bool  g_mousePress;
bool  g_mousePressLeft;
bool  g_mousePressRight;
bool  g_mousePressMiddle;
bool  g_mousePressOverride{false};
bool  g_mouseLockInPlace{false};
bool  g_windowFocusFlag;
bool  g_skipRendering;
bool  g_useVorticityConfinement{false};
bool  g_imGuiButton[15]{false};
u8    g_runCodeOnce[3];
u8    g_chooseTextureToRender{TEXTURE_DRAW_VELOCITY_PRESSURE};
u8    g_chooseUserDrawFillType{USER_DRAW_FILL_TYPE_NONE};
f32   g_textureHighlightSmallValue{100.0f};
f32   g_splatterRadius{0.009};
f32   g_maxSpectralRadius{0};
f32   g_iterationErrorN{0};
vec4f g_mousedxdy{0.0f, 1.0f, 0.0f, 0.0f};
vec2f g_mousePosition{g_windowSize.x / 2, g_windowSize.y / 2};
vec2f g_maxVelocity{0.0f, 0.0f};
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
vec4f g_splatterColor{1.0f, 1.0f, 1.0f, 1.0f};
matrixView<vec4f> g_boundaryTextureData{
    __rcast(vec4f*, util::aligned_malloc<sizeof(vec4f)>(g_dims.x * g_dims.y * sizeof(vec4f)) ),
    g_dims.x,
    g_dims.y
};


/* OpenGL Data */
gl::GLsync         g_fence; 
u32                g_texture[21];
u32                g_texture3D[2];
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
const u32& gr_bndTexture0   = g_texture[17];
const u32& gr_bndTexture1   = g_texture[18];
const u32& gr_bndQuantity0  = g_texture3D[0];
const u32& gr_bndQuantity1  = g_texture3D[1];
const u32& g_reductionErrBuffer = g_persistentbuf[0];
const u32& g_reductionMaxBuffer = g_persistentbuf[1];
void* g_reductionMaxMappedBuf = nullptr;
void* g_reductionErrMappedBuf = nullptr;




static void custom_mousebutton_callback(AWC2User_callback_mousebutton_struct const* data);
static void custom_winfocus_callback(AWC2User_callback_winfocus_struct const* data);
static void custom_winsize_callback(AWC2User_callback_winsize_struct const*);
static void custom_cursor_callback(AWC2User_callback_mousecursor_struct const* data);


void initializeLibrary()
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
        __scast(u16, g_windowSize.x),
        __scast(u16, g_windowSize.y),
        desc
    };
    awc2WindowDescriptorDefault(&ctxtinfo.winDesc);
    awc2initializeContext(&ctxtinfo);
    awc2setContextUserCallbackMouseButton  (g_contextid, &custom_mousebutton_callback);
    awc2setContextUserCallbackWindowFocus  (g_contextid, &custom_winfocus_callback);
    awc2setContextUserCallbackWindowSize   (g_contextid, &custom_winsize_callback);
    awc2setContextUserCallbackMousePosition(g_contextid, &custom_cursor_callback);
    markstr("AWC2 init end");
    return;
}


void destroyLibrary()
{
    markstr("AWC2 Destroy Begin");
    awc2destroyContext(g_contextid);
    awc2destroy();
    markstr("AWC2 Destroy End");
    return;
}




void initializeGraphics()
{
    u8 alive{true};


    markstr("Graphics Init Begin");
    gl::glCreateTextures(gl::GL_TEXTURE_2D, __carraysize(g_texture), &g_texture[0]);
    gl::glCreateTextures(gl::GL_TEXTURE_3D, __carraysize(g_texture3D), &g_texture3D[0]);
    gl::glCreateBuffers(__carraysize(g_persistentbuf), &g_persistentbuf[0]);
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


    auto& tex = g_texture[0];
    for(u32 i = 0; i < __carraysize(g_texture); ++i) {
        tex = g_texture[i];
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_WRAP_S, gl::GL_CLAMP_TO_EDGE);
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_WRAP_T, gl::GL_CLAMP_TO_EDGE);
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_MIN_FILTER, gl::GL_NEAREST);
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_MAG_FILTER, gl::GL_NEAREST);
        gl::glTextureStorage2D(tex, 1, gl::GL_RGBA32F, g_dims.x, g_dims.y);
        gl::glClearTexImage(tex, 0, gl::GL_RGBA, gl::GL_FLOAT, nullptr);
    }
    gl::glTextureParameteri(gr_bndQuantity0, gl::GL_TEXTURE_WRAP_S, gl::GL_CLAMP_TO_EDGE);
    gl::glTextureParameteri(gr_bndQuantity0, gl::GL_TEXTURE_WRAP_T, gl::GL_CLAMP_TO_EDGE);
    gl::glTextureParameteri(gr_bndQuantity0, gl::GL_TEXTURE_WRAP_R, gl::GL_CLAMP_TO_EDGE);
    gl::glTextureParameteri(gr_bndQuantity0, gl::GL_TEXTURE_MIN_FILTER, gl::GL_NEAREST);
    gl::glTextureParameteri(gr_bndQuantity0, gl::GL_TEXTURE_MAG_FILTER, gl::GL_NEAREST);
    gl::glTextureStorage3D(gr_bndQuantity0, 1, gl::GL_RGBA32F, g_dims.x, g_dims.y, 4);
    gl::glClearTexImage(gr_bndQuantity0, 0, gl::GL_RGBA, gl::GL_FLOAT, nullptr);

    gl::glTextureParameteri(gr_bndQuantity1, gl::GL_TEXTURE_WRAP_S, gl::GL_CLAMP_TO_EDGE);
    gl::glTextureParameteri(gr_bndQuantity1, gl::GL_TEXTURE_WRAP_T, gl::GL_CLAMP_TO_EDGE);
    gl::glTextureParameteri(gr_bndQuantity1, gl::GL_TEXTURE_WRAP_R, gl::GL_CLAMP_TO_EDGE);
    gl::glTextureParameteri(gr_bndQuantity1, gl::GL_TEXTURE_MIN_FILTER, gl::GL_NEAREST);
    gl::glTextureParameteri(gr_bndQuantity1, gl::GL_TEXTURE_MAG_FILTER, gl::GL_NEAREST);
    gl::glTextureStorage3D(gr_bndQuantity1, 1, gl::GL_RGBA32F, g_dims.x, g_dims.y, 4);
    gl::glClearTexImage(gr_bndQuantity1, 0, gl::GL_RGBA, gl::GL_FLOAT, nullptr);


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


    for(i32 i = 0; i < g_dims.x; ++i) {
        for(i32 j = 0; j < g_dims.j; ++j) {
            if( (i == 0 || i == g_dims.x - 1) || (j == 0 || j == g_dims.y - 1) ) 
                g_boundaryTextureData(i, j) = vec4f{1};
        }
    }
    gl::glTextureSubImage2D(gr_bndTexture0, 0, 
        0, 0, g_dims.x, g_dims.y, 
        gl::GL_RGBA, gl::GL_FLOAT, 
        g_boundaryTextureData.m_buf
    );
    gl::glTextureSubImage2D(gr_bndTexture1, 0, 
        0, 0, g_dims.x, g_dims.y, 
        gl::GL_RGBA, gl::GL_FLOAT, 
        g_boundaryTextureData.m_buf
    );


    gl::glNamedFramebufferTexture(g_fbo, gl::GL_COLOR_ATTACHMENT0, gr_drawTexture, 0);
    ifcrash( gl::glCheckNamedFramebufferStatus(g_fbo, gl::GL_FRAMEBUFFER) 
        != gl::GL_FRAMEBUFFER_COMPLETE 
    );


    markstr("Graphics Init End  ");
    return;
}


void destroyGraphics()
{
    markstr("Graphics Destroy Begin");

    gl::glDeleteTextures(__carraysize(g_texture3D), &g_texture3D[0]);
    gl::glDeleteTextures(__carraysize(g_texture), &g_texture[0]);
    gl::glUnmapNamedBuffer(g_reductionMaxBuffer);
    gl::glUnmapNamedBuffer(g_reductionErrBuffer);
    gl::glDeleteBuffers(__carraysize(g_persistentbuf), &g_persistentbuf[0]);
    gl::glDeleteFramebuffers(1, &g_fbo);
    for(auto& comp : g_compute) {
        comp.destroy();
    }
    util::aligned_free(g_boundaryTextureData.m_buf);


    markstr("Graphics Destroy End");
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
    
    g_mousePressLeft   = awc2isMouseButtonPressed(AWC2_MOUSEBUTTON_LEFT);
    g_mousePressRight  = awc2isMouseButtonPressed(AWC2_MOUSEBUTTON_RIGHT);
    g_mousePressMiddle = awc2isMouseButtonPressed(AWC2_MOUSEBUTTON_MIDDLE);
    g_mousePress       = g_mousePressLeft || g_mousePressRight || g_mousePressMiddle;
    if(!g_mouseLockInPlace)
        g_mousePressOverride = g_mousePressRight;
    return;
}


static void custom_winfocus_callback(AWC2User_callback_winfocus_struct const* data) 
{
    g_windowFocusFlag = data->focused;
    return;
}


static void custom_winsize_callback(AWC2User_callback_winsize_struct const* data)
{
    g_windowSize = vec2i{ 
        __scast(u32, data->width), 
        __scast(u32, data->height) 
    };
    if(g_mouseLockInPlace) {
        g_mousePosition = vec2f{g_windowSize.x, g_windowSize.y} * 0.5f;
    }
    return;
}


static void custom_cursor_callback(AWC2User_callback_mousecursor_struct const* data)
{
    if(!g_mouseLockInPlace) {
        auto mousedelta = awc2getMousePositionDelta();
        g_mousePosition = vec2f{ data->pos.x, data->pos.y };
        g_mousedxdy     = vec4f{ mousedelta.x, mousedelta.y, 0, 0 };
    }
    return;
}


} /* namespace 23multigrid */
