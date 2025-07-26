#pragma once
#include <util2/time.hpp>
#include <util2/vec2.hpp>
#include <glbinding/gl46core/gl.h>
#include "gl/shader2.hpp"


namespace features18 {


using namespace util2::math;


extern const char* computeShaderFilename[8];
extern u8                 g_measureTimeOnce;
extern u8                 g_runCodeOnceFlag;
extern u8                 g_contextid;
extern util2::Time::Timestamp    g_frameTime;
extern util2::Time::Timestamp    g_measuremisc[3];
extern util2::Time::Timestamp    g_renderTime;
extern i64                g_minFrameTimeNs;
extern i64                g_maxFrameTimeNs;
extern i64                g_avgFrameTimeNs;
extern i64                g_slowRenderWaitDurationNs;
extern i64                g_waitTime;
extern util2::Time::Timestamp    g_computeDyeTime;
extern util2::Time::Timestamp    g_computeVelTime;
extern util2::Time::Timestamp    g_computeCFLTime;
extern util2::Time::Timestamp    g_computeFluidTime;
extern util2::Time::Timestamp    g_computeMaximumCPU;
extern util2::Time::Timestamp    g_computeMaximumGPU;
extern util2::Time::Timestamp    g_renderImguiTime;
extern util2::Time::Timestamp    g_renderScreenTime;
extern util2::Time::Timestamp    g_refreshShaderTime;


extern std::vector<vec4f> g_initialField;
extern vec2i              g_dims;
extern vec2i              g_windims;
extern vec3u              g_localWorkGroupSize;
extern vec3u              g_computeInvocationSize;
extern vec2f              g_simUnitCoords;
extern f32                g_dt;
extern f32                g_normdt;
extern f32                g_viscosity;
extern f32                g_viscosity;
extern f32                g_dampFactor;
extern f32                g_vorticityConfinement;
extern f32                g_buoyancy;
extern f32                g_ambientTemperature;
extern i32                g_maximumJacobiIterations;
extern i32                g_reductionFactor;
extern i32                g_reductionBufferLength;
extern std::vector<vec4f> g_reductionBuffer;
extern vec2f              g_maxVelocity;

extern gl::GLsync         g_fence; 
extern u32                g_texture[19];
extern u32                g_persistentbuf[2];
extern void*              g_mappedBuffer[2];
extern u32                g_fbo;
extern ShaderProgramV2    g_compute[__carraysize(computeShaderFilename)];
extern vec2f              g_mouseDragForce;
extern vec2f              g_mouseDragPosition;
extern f32                g_splatterRadius;
extern vec4f              g_splatterForce;
extern vec4f              g_splatterColor;
extern bool               g_windowFocus;
extern bool               g_mousePressed;
extern bool               g_mouseDrawEvent;
extern bool               g_slowRender;
extern bool               g_skipRender;
extern bool               g_buttonPressed[3];
extern bool               g_useVorticityConfinement;
extern u32                g_whatToRender;
extern u32                g_slowRenderCounter;
extern u32                g_frameCounter;


extern ShaderProgramV2& gr_computeInteractive      ;
extern ShaderProgramV2& gr_computeAdvection        ;
extern ShaderProgramV2& gr_computeDiffusionVel     ;
extern ShaderProgramV2& gr_computeAdd              ;
extern ShaderProgramV2& gr_computeDivergence       ;
extern ShaderProgramV2& gr_computeDiffusionPressure;
extern ShaderProgramV2& gr_computeNewVelocity      ;
extern ShaderProgramV2& gr_computeRenderToTex      ;
extern ShaderProgramV2& gr_computeCFLCondition     ;


extern u32&             gr_drawTexture;
extern u32&             gr_dyeTexture0;
extern u32&             gr_dyeTexture1;
extern u32&             gr_tmpTexture0;
extern u32&             gr_tmpTexture1;

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
extern u32& g_reductionMinTexture;
extern u32& g_reductionMaxTexture;
extern void* g_reductionMaxMappedBuf;


inline u8               getContextID()      { return g_contextid;      }
inline u64              getAvgFrameTimeNs() { return g_avgFrameTimeNs; }
inline u32&             getFrameCounter()   { return g_frameCounter;   }
inline util2::Time::Timestamp& getFrameTime()      { return g_frameTime;      }
inline util2::Time::Timestamp& getRenderTime()     { return g_renderTime;     }
inline util2::Time::Timestamp& getTimer0()         { return g_measuremisc[0]; }
inline util2::Time::Timestamp& getTimer1()         { return g_measuremisc[1]; }
inline util2::Time::Timestamp& getTimer2()         { return g_measuremisc[2]; }


void initializeLibrary();
void destroyLibrary();
void initializeGraphics();
void destroyGraphics();


}; /* namespace features18 */