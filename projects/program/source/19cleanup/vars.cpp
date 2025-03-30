#include "vars.hpp"
#include <awc2/C/awc2.h>
#include <util/marker2.hpp>


namespace cleanup19 {


// const char* computeShaderFilename[8] = {
//     "shaders/0force.comp",
//     "shaders/1advect.comp",
//     "shaders/2diffuse.comp",
//     "shaders/3div.comp",
//     "shaders/4pressure.comp",
//     "shaders/5final.comp",
//     "shaders/6draw.comp",
//     "shaders/minmax.comp"
// };
const char* computeShaderFilename[8] = {
    "projects/program/source/19cleanup/shaders/0force.comp",
    "projects/program/source/19cleanup/shaders/1advect.comp",
    "projects/program/source/19cleanup/shaders/2diffuse.comp",
    "projects/program/source/19cleanup/shaders/3div.comp",
    "projects/program/source/19cleanup/shaders/4pressure.comp",
    "projects/program/source/19cleanup/shaders/5final.comp",
    "projects/program/source/19cleanup/shaders/6draw.comp",
    "projects/program/source/19cleanup/shaders/minmax.comp"
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
Time::Timestamp    g_computeMaximumCPU{};
Time::Timestamp    g_computeMaximumGPU{};
Time::Timestamp    g_renderImguiTime{};
Time::Timestamp    g_renderScreenTime{};


/* Simulation Constants */
f32   g_kinematicViscosity = 1.0f;
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
bool  g_windowFocusFlag;
bool  g_skipRendering;
bool  g_useVorticityConfinement{false};
bool  g_imGuiButton[5]{false};
u8    g_runCodeOnce[3];
u8    g_chooseTextureToRender;
f32   g_textureHighlightSmallValue{1.0f};
f32   g_splatterRadius{0.009};
vec2f g_mousedxdy{0, 0};
vec2f g_mousePosition{0, 0};
vec2f g_maxVelocity{0.0f, 0.0f};
vec4f g_splatterForce{1.0f, 1.0f, 1.0f, 1.0f};
vec4f g_splatterColor{1.0f, 1.0f, 1.0f, 1.0f};


vec2i              g_dims{1920, 1080};
vec2i              g_windowSize{g_dims};
vec3u              g_localWorkGroupSize = { 64, 1, 1 };
vec3u              g_computeInvocationSize = vec3u{ g_dims.x, g_dims.y, 1 } / g_localWorkGroupSize;
i32                g_maximumJacobiIterations = 80;
i32                g_reductionFactor = 4096;
i32                g_reductionBufferLength = g_dims.x * g_dims.y / g_reductionFactor;
std::vector<vec4f> g_reductionBuffer;

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


u32& gr_drawTexture   = g_texture[0];
u32& gr_dyeTexture0   = g_texture[1];
u32& gr_dyeTexture1   = g_texture[2];
u32& gr_tmpTexture0   = g_texture[3];
u32& gr_tmpTexture1   = g_texture[4];
u32& gr_outTexShader0 = g_texture[5];
u32& gr_outTexShader1 = g_texture[6];
u32& gr_outTexShader2 = g_texture[7]; /* will use tmpTexture for ping-pong */
u32& gr_outTexShader3 = g_texture[8];
u32& gr_outTexShader4 = g_texture[9];
u32& gr_outTexShader5 = g_texture[10];
u32& gr_outTexShader6 = g_texture[11];
u32& gr_outTexShader7 = g_texture[12];
u32& gr_outTexShader8 = g_texture[13];
u32& gr_outTexShader9 = g_texture[14];
u32& gr_simTexture0   = g_texture[15]; /* Components are [u.x, u.y, p, reserved] */
u32& gr_simTexture1   = g_texture[16];
u32& g_reductionMinTexture = g_persistentbuf[0];
u32& g_reductionMaxTexture = g_persistentbuf[1];
void* g_reductionMaxMappedBuf = nullptr;




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
        __scast(u16, g_dims.x),
        __scast(u16, g_dims.y),
        desc
    };
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


    for(u32 i = 0; i < __carraysize(g_texture); ++i) {
        auto& tex = g_texture[i];
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_WRAP_S, gl::GL_CLAMP_TO_EDGE);
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_WRAP_T, gl::GL_CLAMP_TO_EDGE);
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_MIN_FILTER, gl::GL_NEAREST);
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_MAG_FILTER, gl::GL_NEAREST);
        gl::glTextureStorage2D(tex, 1, gl::GL_RGBA32F, g_dims.x, g_dims.y);
        gl::glClearTexImage(tex, 0, gl::GL_RGBA, gl::GL_FLOAT, nullptr);
    }


    gl::glNamedBufferStorage(g_reductionMaxTexture, 
        g_reductionBufferLength * sizeof(vec4f), 
        nullptr, 
        gl::GL_NONE_BIT
        | gl::GL_MAP_READ_BIT
        | gl::GL_MAP_PERSISTENT_BIT
        | gl::GL_MAP_COHERENT_BIT
    );
    g_reductionMaxMappedBuf = gl::glMapNamedBufferRange(g_reductionMaxTexture, 
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


void destroyGraphics()
{
    markstr("Graphics Destroy Begin");


    gl::glDeleteTextures(__carraysize(g_texture), &g_texture[0]);
    gl::glUnmapNamedBuffer(g_reductionMaxTexture);
    gl::glDeleteBuffers(__carraysize(g_persistentbuf), &g_persistentbuf[0]);
    gl::glDeleteFramebuffers(1, &g_fbo);
    for(auto& comp : g_compute) {
        comp.destroy();
    }
    g_reductionBuffer.resize(0);
    

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
    return;
}


static void custom_cursor_callback(AWC2User_callback_mousecursor_struct const* data)
{
    auto mousedelta = awc2getMousePositionDelta();
    g_mousePosition = vec2f{ data->pos.x  , data->pos.y };
    g_mousedxdy     = vec2f{ mousedelta.x, mousedelta.y };
    return;
}


} /* namespace cleanup19 */
