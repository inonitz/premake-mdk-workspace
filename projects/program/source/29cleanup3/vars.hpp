#pragma once
#include <util/time.hpp>
#include <util/vec2.hpp>
#include <glbinding/gl/types.h>
#include "gl/shader2.hpp"
#include "gl/gltimer.hpp"


namespace cleanup329 {


using namespace util::math;


enum class FillType : u8 {
    NONE           = 0x00,
    DYE            = 0x01,
    FORCE          = 0x02,
    FORCE_AND_DYE  = 0x03,
    BOUNDARY       = 0x04,
    FILL_TYPE_MAX  = 0x05
};


inline const char* fillTypeToString(FillType type)
{
    static constexpr const char* strs[6] = {
        "NONE         ",
        "DYE          ",
        "FORCE        ",
        "FORCE_AND_DYE",
        "BOUNDARY     ",
        "FILL_TYPE_MAX"
    };
    return strs[__scast(u8, type)];
}


enum class DrawTarget : u8 {
    DYE               = 0x00,
    VELOCITY_PRESSURE = 0x01,
    CURL              = 0x02,
    ABS_CURL          = 0x03,
    ERROR_VELOCITY    = 0x04,
    ERROR_PRESSURE    = 0x05,
    BOUNDARY          = 0x06,
    VELOCITY_X        = 0x07,
    VELOCITY_Y        = 0x08,
    PREESURE          = 0x09,
    CFL_CONTOUR       = 0x0A,
    DRAW_TARGET_MAX   = 0x0B
};


enum class ColourMap : u8 {
    INFERNO = 0,
    VIRDIS  = 1,
    FAST    = 2
};


struct FluidSource {
    bool     m_enabled;
    bool     m_static;
    FillType m_type;
    u8       m_reserved[1];
    f32      m_radius;
    vec2f    m_position;
    vec4f    m_force;
    vec4f    m_color;
};


extern const char* computeShaderFilename[10];


/* 
    Time Measurements / Constants
*/
extern u8                 g_contextid;
extern u32                g_frameCounter;
extern i64                g_minFrameTimeNs;
extern i64                g_maxFrameTimeNs;
extern i64                g_avgFrameTimeNs;
extern const i64          g_slowRenderDurationNs;
extern i64                g_waitTime;
extern Time::Timestamp    g_cpuTimerBuffer[16];
extern Time::GPUTimer     g_gpuTimerBuffer[5];

extern Time::Timestamp&   g_frameTime             ;
extern Time::Timestamp&   g_beginFrameTime        ;
extern Time::Timestamp&   g_fluidUpdateTime       ;
extern Time::Timestamp&   g_computeVelTime        ;
extern Time::Timestamp&   g_computeDyeTime        ;
extern Time::Timestamp&   g_computeCFLTime        ;
extern Time::Timestamp&   g_computeErrEstimateTime;
extern Time::Timestamp&   g_renderScreenTime      ;
extern Time::Timestamp&   g_renderImGuiTime       ;
extern Time::Timestamp&   g_renderBlitTime        ;
extern Time::Timestamp&   g_endFrameTime          ;
extern Time::Timestamp&   g_computeMaximumCPU     ;
extern Time::Timestamp&   g_computeMaximumGPU     ;
extern Time::Timestamp&   g_computeErrorGPU       ;
extern Time::Timestamp&   g_computeErrorCPU       ;
extern Time::GPUTimer&    g_computeVelTimeGPU     ;
extern Time::GPUTimer&    g_computeDyeTimeGPU     ;
extern Time::GPUTimer&    g_computeCFLTimeGPU     ;
extern Time::GPUTimer&    g_computeErrTimeGPU     ;
extern Time::GPUTimer&    g_computeScreenTimeGPU  ;


/* Compute Parameters */
extern const vec2i        g_dims;
extern const vec2f        g_invdims;
extern const vec3u        g_localWorkGroupSize;
extern const vec3u        g_computeInvocationSize;
extern i32                g_maximumJacobiIterations;
extern const i32          g_reductionFactor;
extern const i32          g_reductionBufferLength;


/* Simulation Constants */
extern f32   g_kinematicViscosity;
extern f32   g_densityPressure   ;
extern f32   g_confineVorticity  ;
extern f32   g_buoyancyStrength  ;
extern f32   g_ambientTemperature;
extern f32   g_dt                ;
extern f32   g_normdt            ;
extern f32   g_cfl               ;
extern f32   g_reynolds          ;
extern f32   g_unitLength        ;


/* User Interaction/Info */
extern std::vector<FluidSource> g_sources;
extern bool       g_imGuiButton[20];
extern bool       g_enableGPUTimers;
extern u64        g_mostRecentSource;
extern DrawTarget g_chooseTextureToRender;
extern ColourMap  g_chooseColourMap;
extern f32        g_textureHighlightSmallValue;
extern f32        g_maxSpectralRadius;
extern f32        g_iterationErrorN;
extern vec2f      g_maxVelocity;
extern vec2f      g_colorTableMinMax;
extern vec4f      g_prevErrorValues[3];
extern vec4f      g_currErrorValues[3];


/* OpenGL Data */
extern gl::GLsync      g_fence;
extern u32             g_texture[19];
extern u32             g_persistentbuf[2];
extern void*           g_mappedBuffer[2];
extern u32             g_fbo;
extern ShaderProgramV2 g_compute[__carraysize(computeShaderFilename)];



extern ShaderProgramV2& gr_computeBoundaries       ;
extern ShaderProgramV2& gr_computeInteractive      ;
extern ShaderProgramV2& gr_computeAdvection        ;
extern ShaderProgramV2& gr_computeDiffusionVel     ;
extern ShaderProgramV2& gr_computeDivergence       ;
extern ShaderProgramV2& gr_computeDiffusionPressure;
extern ShaderProgramV2& gr_computeNewVelocity      ;
extern ShaderProgramV2& gr_computeRenderToTex      ;
extern ShaderProgramV2& gr_computeCFLCondition     ;
extern ShaderProgramV2& gr_computeErrorTexture     ;
extern ShaderProgramV2& gr_computeErrorEstimates   ;


extern const u32& gr_drawTexture;
extern const u32& gr_dyeTexture0;
extern const u32& gr_dyeTexture1;
extern const u32& gr_tmpTexture0; /* will use tmpTexture for ping-pong */
extern const u32& gr_tmpTexture1;
extern const u32& gr_outTexShader0;
extern const u32& gr_outTexShader1;
extern const u32& gr_outTexShader2;
extern const u32& gr_outTexShader3;
extern const u32& gr_outTexShader4;
extern const u32& gr_outTexShader5;
extern const u32& gr_outTexShader6;
extern const u32& gr_outTexShader7;
extern const u32& gr_outTexShader8;
extern const u32& gr_outTexShader9;
extern const u32& gr_simTexture0; /* Components are [u.x, u.y, p, reserved] */
extern const u32& gr_simTexture1;
extern const u32& gr_bndQuantity0;
extern const u32& gr_bndQuantity1;
extern const u32& g_reductionErrBuffer;
extern const u32& g_reductionMaxBuffer;
extern const u32& gr_errorTexture;
extern u32 gr_prevIterVel;
extern u32 gr_nextIterVel;
extern u32 gr_prevIterDye;
extern u32 gr_prevIterDye;
extern u32 gr_nextIterDye;
extern void* g_reductionMaxMappedBuf;
extern void* g_reductionErrMappedBuf;




namespace init {

    inline u8 getContextID() { return g_contextid; }
    void initializeLibrary();
    void destroyLibrary();
    void initializeGraphics();
    void destroyGraphics();


} /* namespace cleanup329::init */


}; /* namespace cleanup329 */