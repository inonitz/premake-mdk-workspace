#pragma once
#include <util2/time.hpp>
#include <util2/vec2.hpp>
#include <glbinding/gl46core/gl.h>
#include "gl/shader2.hpp"


namespace anotherattempt26 {


using namespace util2::math;
#define USER_DRAW_FILL_TYPE_NONE          0x00
#define USER_DRAW_FILL_TYPE_DYE           0x01
#define USER_DRAW_FILL_TYPE_FORCE         0x02
#define USER_DRAW_FILL_TYPE_BOUNDARY      0x03
#define USER_DRAW_FILL_TYPE_FORCE_AND_DYE 0x04
#define TEXTURE_DRAW_DYE               0x00
#define TEXTURE_DRAW_VELOCITY_PRESSURE 0x01
#define TEXTURE_DRAW_CURL              0x02
#define TEXTURE_DRAW_ABS_CURL          0x03
#define TEXTURE_DRAW_ERROR_VELOCITY    0x04
#define TEXTURE_DRAW_ERROR_PRESSURE    0x05
#define TEXTURE_DRAW_BOUNDARY          0x06
#define TEXTURE_DRAW_VELOCITY_X        0x07
#define TEXTURE_DRAW_VELOCITY_Y        0x08
#define TEXTURE_DRAW_PREESURE          0x09
#define TEXTURE_DRAW_BOUNDARY_VALUE_VEL      0x0A
#define TEXTURE_DRAW_BOUNDARY_VALUE_PRESSURE 0x0B


extern const char* computeShaderFilename[11];


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
extern util2::Time::Timestamp    g_frameTime;
extern util2::Time::Timestamp    g_renderTime;
extern util2::Time::Timestamp    g_beginFrameTime;
extern util2::Time::Timestamp    g_endFrameTime;
extern util2::Time::Timestamp    g_computeFluidTime;
extern util2::Time::Timestamp    g_computeVelTime;
extern util2::Time::Timestamp    g_computeDyeTime;
extern util2::Time::Timestamp    g_computeCFLTime;
extern util2::Time::Timestamp    g_computeErrEstimateTime;
extern util2::Time::Timestamp    g_computeMaximumCPU;
extern util2::Time::Timestamp    g_computeMaximumGPU;
extern util2::Time::Timestamp    g_computeErrorGPU;
extern util2::Time::Timestamp    g_computeErrorCPU;
extern util2::Time::Timestamp    g_renderImguiTime;
extern util2::Time::Timestamp    g_renderScreenTime;


/* Compute Parameters */
extern const vec2i        g_dims;
extern const vec2f        g_invdims;
extern vec2i              g_windowSize;
extern const vec3u        g_localWorkGroupSize;
extern const vec3u        g_computeInvocationSize;
extern i32                g_maximumJacobiIterations;
extern const i32          g_reductionFactor;
extern const i32          g_reductionBufferLength;


/* Simulation Constants */
extern f32   g_kinematicViscosity;
extern f32   g_densityPressure;
extern f32   g_confineVorticity  ;
extern f32   g_buoyancyStrength  ;
extern f32   g_ambientTemperature;
extern f32   g_dt                ;
extern f32   g_normdt            ;
extern f32   g_cfl               ;
extern vec2f g_simUnitCoords     ;


/* User Interaction/Info */
extern bool  g_mousePress;
extern bool  g_mousePressLeft;
extern bool  g_mousePressRight;
extern bool  g_mousePressMiddle;
extern bool  g_mousePressOverride;
extern bool  g_mouseLockInPlace;
extern bool  g_windowFocusFlag;
extern bool  g_skipRendering;
extern bool  g_useVorticityConfinement;
extern bool  g_useShaderNoBoundaries[2];
extern bool  g_imGuiButton[20];
extern u8    g_runCodeOnce[3];
extern u8    g_chooseTextureToRender;
extern f32   g_textureHighlightSmallValue;
extern u8    g_chooseUserDrawFillType;
extern f32   g_splatterRadius;
extern f32   g_maxSpectralRadius;
extern f32   g_iterationErrorN;
extern vec4f g_mousedxdy;
extern vec2f g_mousePosition;
extern vec2f g_maxVelocity;
extern vec4f g_prevErrorValues[3];
extern vec4f g_currErrorValues[3];
extern vec4f g_splatterColor;


/* OpenGL Data */
extern gl::GLsync         g_fence; 
extern u32                g_texture[19];
extern u32                g_persistentbuf[2];
extern void*              g_mappedBuffer[2];
extern u32                g_fbo;
extern ShaderProgramV2    g_compute[__carraysize(computeShaderFilename)];



extern ShaderProgramV2& gr_computeBoundaries        ;
extern ShaderProgramV2& gr_computeInteractive       ;
extern ShaderProgramV2& gr_computeAdvection         ;
extern ShaderProgramV2& gr_computeDiffusionVel      ;
extern ShaderProgramV2& gr_computeDivergence        ;
extern ShaderProgramV2& gr_computeDiffusionPressure ;
extern ShaderProgramV2& gr_computeNewVelocity       ;
extern ShaderProgramV2& gr_computeRenderToTex       ;
extern ShaderProgramV2& gr_computeCFLCondition      ;
extern ShaderProgramV2& gr_computeErrorTexture      ;
extern ShaderProgramV2& gr_computeErrorEstimates    ;


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
extern const u32& gr_bndryTexture0;
extern const u32& gr_bndryTexture1;
extern const u32& gr_bndQuantity0;
extern const u32& gr_bndQuantity1;
extern const u32& g_reductionErrBuffer;
extern const u32& g_reductionMaxBuffer;
extern const u32& gr_errorTexture;
extern u32 gr_prevIterBounds;
extern u32 gr_nextIterBounds;
extern u32 gr_bndQtyVel     ;
extern u32 gr_bndQtyPressure;
extern u32 gr_prevIterVel;
extern u32 gr_nextIterVel;
extern u32 gr_prevIterDye;
extern u32 gr_prevIterDye;
extern u32 gr_nextIterDye;
extern void* g_reductionMaxMappedBuf;
extern void* g_reductionErrMappedBuf;


inline u8 getContextID() { return g_contextid; }
void clearSimulationTextures();
void initializeLibrary();
void destroyLibrary();
void initializeGraphics();
void destroyGraphics();


}; /* namespace cleanup19 */