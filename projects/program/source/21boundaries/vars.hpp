#pragma once
#include <util2/time.hpp>
#include <util2/vec2.hpp>
#include <glbinding/gl46core/gl.h>
#include "gl/shader2.hpp"


namespace boundary21 {


using namespace util2::math;


extern const char* computeShaderFilename[8];


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
extern util2::Time::Timestamp    g_computeMaximumCPU;
extern util2::Time::Timestamp    g_computeMaximumGPU;
extern util2::Time::Timestamp    g_renderImguiTime;
extern util2::Time::Timestamp    g_renderScreenTime;


/* Simulation Constants */
extern f32   g_kinematicViscosity;
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
extern bool  g_windowFocusFlag;
extern bool  g_skipRendering;
extern bool  g_useVorticityConfinement;
extern bool  g_imGuiButton[10];
extern u8    g_runCodeOnce[3];
extern u8    g_chooseTextureToRender;
extern f32   g_textureHighlightSmallValue;
extern u8    g_chooseUserDrawFillType;
extern f32   g_splatterRadius;
extern vec2f g_mousedxdy;
extern vec2f g_mousePosition;
extern vec2f g_maxVelocity;
extern vec4f g_splatterForce;
extern vec4f g_splatterColor;


extern vec2i              g_dims;
extern vec2i              g_windowSize;
extern vec3u              g_localWorkGroupSize;
extern vec3u              g_computeInvocationSize;
extern i32                g_maximumJacobiIterations;
extern i32                g_reductionFactor;
extern i32                g_reductionBufferLength;
extern std::vector<vec4f> g_reductionBuffer;

extern gl::GLsync         g_fence; 
extern u32                g_texture[19];
extern u32                g_persistentbuf[2];
extern void*              g_mappedBuffer[2];
extern u32                g_fbo;
extern ShaderProgramV2    g_compute[__carraysize(computeShaderFilename)];



extern ShaderProgramV2& gr_computeInteractive      ;
extern ShaderProgramV2& gr_computeAdvection        ;
extern ShaderProgramV2& gr_computeDiffusionVel     ;
extern ShaderProgramV2& gr_computeAdd              ;
extern ShaderProgramV2& gr_computeDivergence       ;
extern ShaderProgramV2& gr_computeDiffusionPressure;
extern ShaderProgramV2& gr_computeNewVelocity      ;
extern ShaderProgramV2& gr_computeRenderToTex      ;
extern ShaderProgramV2& gr_computeCFLCondition     ;


extern u32& gr_drawTexture;
extern u32& gr_dyeTexture0;
extern u32& gr_dyeTexture1;
extern u32& gr_tmpTexture0;
extern u32& gr_tmpTexture1;
extern u32& gr_outTexShader0;
extern u32& gr_outTexShader1;
extern u32& gr_outTexShader2; /* will use tmpTexture for ping-pong */
extern u32& gr_outTexShader3;
extern u32& gr_outTexShader4;
extern u32& gr_outTexShader5;
extern u32& gr_outTexShader6;
extern u32& gr_outTexShader7;
extern u32& gr_outTexShader8;
extern u32& gr_outTexShader9;
extern u32& gr_simTexture0; /* Components are [u.x, u.y, p, reserved] */
extern u32& gr_simTexture1;
extern u32& gr_bndryTexture0;
extern u32& gr_bndryTexture1;
extern u32& g_reductionMinTexture;
extern u32& g_reductionMaxTexture;
extern void* g_reductionMaxMappedBuf;


inline u8 getContextID() { return g_contextid; }
void initializeLibrary();
void destroyLibrary();
void initializeGraphics();
void destroyGraphics();


}; /* namespace cleanup19 */