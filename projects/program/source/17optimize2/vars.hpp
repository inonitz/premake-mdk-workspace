#pragma once
#include <util/time.hpp>
#include <util/vec2.hpp>
#include <util/marker2.hpp>
#include <glbinding/gl46core/gl.h>
#include "gl/shader2.hpp"


namespace optimize17 {


using namespace util::math;


extern const char* computeShaderFilename[9];
extern u8                 g_runCodeOnceFlag;
extern u8                 g_contextid;
extern Time::Timestamp    g_frameTime;
extern Time::Timestamp    g_measuremisc0;
extern Time::Timestamp    g_measuremisc1;
extern Time::Timestamp    g_renderTime;
extern f32                g_minRenderTime;
extern f32                g_maxRenderTime;
extern f32                g_avgRenderTime;
extern Time::Timestamp    g_computeVelTime;
extern Time::Timestamp    g_computeCFLTime;
extern Time::Timestamp    g_computeFluidTime;
extern Time::Timestamp    g_retrieveTextureData;
extern Time::Timestamp    g_computeMaximum;
extern Time::Timestamp    g_renderImguiTime;
extern Time::Timestamp    g_renderScreenTime;
extern Time::Timestamp    g_refreshShaderTime;


extern std::vector<vec4f> g_initialField;
extern vec2i              g_dims;
extern vec2i              g_windims;
extern vec2f              g_simUnitCoords;
extern f32                g_dt;
extern f32                g_viscosity;
extern f32                g_buoyancy;
extern f32                g_ambientTemperature;
extern i32                g_maximumJacobiIterations;
extern i32                g_reductionFactor;
extern i32                g_reductionBufferLength;
extern std::vector<vec4f> g_reductionBuffer;
extern vec2f              g_velocityCFL;

extern gl::GLsync         g_fence; 
extern u32                g_texture[19];
extern u32                g_persistentbuf[2];
extern void*              g_mappedBuffer[2];
extern u32                g_fbo;
extern ShaderProgramV2    g_compute[__carraysize(computeShaderFilename)];
extern vec2f              g_mouseDragForce;
extern vec2f              g_mouseDragPosition;
extern f32                g_splatterRadius;
extern vec4f              g_splatterColor;
extern bool               g_windowFocus;
extern bool               g_mousePressed;
extern bool               g_mouseDrawEvent;
extern bool               g_slowRender;
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


inline u8               getContextID()      { return g_contextid;    }
inline Time::Timestamp& getFrameTime()      { return g_frameTime;    }
inline Time::Timestamp& getRenderTime()     { return g_renderTime;   }
inline Time::Timestamp& getTimer0()         { return g_measuremisc0; }
inline Time::Timestamp& getTimer1()         { return g_measuremisc1; }
inline bool             getSlowRenderFlag() { return g_slowRender;   }


void initializeLibrary();
void destroyLibrary();
void initializeGraphics();
void destroyGraphics();


}; /* namespace optimize17 */