#include "fluid.hpp"
#include "vars.hpp"
#include <util/marker2.hpp>
#include <util/random.hpp>
#include <glbinding/gl/gl.h>
#include <awc2/C/awc2.h>
#include <immintrin.h>


using namespace cleanup329;


static void compute_velocity();
static void compute_dye();
static void compute_cfl_new(u32 texture);
static void compute_error(
    u32 previousVelocity,
    u32 currentVelocity
);
static void compute_postprocess();


static void invokeCompute(gl::MemoryBarrierMask mask = gl::GL_ALL_BARRIER_BITS);
static u32 addForces(FillType type, u32 interactTexIn);
static u32 advect(u32 previousStep, bool keepOnlyVelocity, u32 outTex);
static u32 diffuseVelocity(u32 previousStep);
static u32 diffusePressure(u32 previousStep);
static void updateVelocities(u32 newvel, u32 newpressure);


// a for lanes where a < b, b otherwise
inline __m128 compareLessThanXY_M128( __m128 a, __m128 b)
{
    const __m128 cmp = _mm_cmplt_ps( a, b );
    return _mm_blendv_ps( b, a, cmp );
}

// a for lanes where a > b, b otherwise
inline __m128 compareBiggerThanXY_M128( __m128 a, __m128 b)
{
    const __m128 cmp = _mm_cmpgt_ps( a, b );
    return _mm_blendv_ps( b, a, cmp );
}


inline __m256 compareBiggerThanXY_M256(__m256 a, __m256 b)
{
    const __m256 cmp = _mm256_cmp_ps(a, b, _CMP_GT_OQ);
    return _mm256_blendv_ps( b, a, cmp);
}

inline __m256 compareLessThanXY_M256(__m256 a, __m256 b)
{
    const __m256 cmp = _mm256_cmp_ps(a, b, _CMP_LT_OQ);
    return _mm256_blendv_ps( b, a, cmp);
}





void fluid::clear()
{
    for(auto& tex : g_texture) {
        gl::glClearTexImage(tex, 0, gl::GL_RGBA, gl::GL_FLOAT, nullptr);
    }
    return;
}


#define TIME_CODE_BLOCK_CUSTOM(enable_gpu_time, counter_cpu, counter_gpu, ...) \
    if (boolean(enable_gpu_time)) \
    { \
        /* No need to mention the GPU-timer overhead on the cpu-side */ \
        /* but i assume its negligeble */ \
        \
        counter_cpu.begin(); \
        Time::GPUTimer::begin(counter_gpu); \
        __VA_ARGS__; \
        Time::GPUTimer::end(counter_gpu); \
        counter_cpu.end(); \
    } else { \
        TIME_NAMESPACE_TIME_CODE_BLOCK(counter_cpu, __VA_ARGS__); \
    } \




void fluid::update()
{
    TIME_CODE_BLOCK_CUSTOM(
        g_enableGPUTimers, 
        g_computeVelTime, 
        g_computeVelTimeGPU,    
        compute_velocity()
    );
    /* 
        Make sure to swap textures for updates to propegate to the next iterations
    */
    std::swap(gr_prevIterVel, gr_nextIterVel);
    /* 
        Subsequent calls to gr_prevIterVel will use the updated field 
    */
    TIME_CODE_BLOCK_CUSTOM(
        g_enableGPUTimers, 
        g_computeDyeTime, 
        g_computeDyeTimeGPU,    
        compute_dye()
    );
    TIME_CODE_BLOCK_CUSTOM(
        g_enableGPUTimers, 
        g_computeCFLTime, 
        g_computeCFLTimeGPU,    
        compute_cfl_new(gr_prevIterVel)
    );
    TIME_CODE_BLOCK_CUSTOM(
        g_enableGPUTimers, 
        g_computeErrEstimateTime, 
        g_computeErrTimeGPU,
        compute_error(gr_nextIterVel, gr_prevIterVel)
    );
    TIME_CODE_BLOCK_CUSTOM(g_enableGPUTimers, 
        g_renderScreenTime, 
        g_computeScreenTimeGPU, 
        compute_postprocess()
    );
    return;
}


void fluid::addSourceDefault()
{
    g_sources.push_back(FluidSource{
        0, 1,
        FillType::FORCE,
        {0},
        0.01f,
        vec2f{0, 0},
        vec4f{0.0f},
        vec4f{ random32f(), random32f(), random32f(), 1.0f }
    });
    return;
}


bool fluid::recompileComputeShaders()
{
    bool alive{true};

    for(uint32_t i = 0; alive && i < __carraysize(computeShaderFilename); ++i) {
        g_compute[i].refreshFromFiles();
        g_compute[i].resizeLocalWorkGroup(0,
            g_localWorkGroupSize.x,
            g_localWorkGroupSize.y,
            g_localWorkGroupSize.z
        );
        alive = alive && g_compute[i].compile();
    }


    return alive;
}




void compute_velocity() 
{
    u32 tmp, pressure;
    tmp = addForces(FillType::FORCE, gr_prevIterVel);
    tmp = advect(tmp, true, gr_outTexShader1);
    tmp = diffuseVelocity(tmp);
    pressure = diffusePressure(tmp);
    updateVelocities(tmp, pressure);
    return;
}


static void compute_dye()
{
    u32 tmp;
    tmp = addForces(FillType::DYE, gr_prevIterDye);
    tmp = advect(tmp, false, gr_nextIterDye);

    std::swap(gr_prevIterDye, gr_nextIterDye);
    return;
}


static void compute_cfl_new(u32 texture)
{
    auto jamAFenceToWaitForGPUBufferVisibility = []() {
        if(g_fence) {
            gl::glDeleteSync(g_fence);
        }
        g_fence = gl::glFenceSync(gl::GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        return;
    };
    g_computeMaximumGPU.begin();
    gl::glBindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, 1, g_reductionMaxBuffer);
    gl::glBindTextureUnit(0, texture);
    gr_computeCFLCondition.bind();
    gr_computeCFLCondition.uniform1i("originalTexture", 0);
    gr_computeCFLCondition.StorageBlock("reductionMaximumBuffer", 1);
    gr_computeCFLCondition.uniform1i("ku_valuesToFetch", g_reductionFactor);
    gl::glDispatchCompute(g_reductionBufferLength, 1, 1);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
    

    jamAFenceToWaitForGPUBufferVisibility();
    g_computeMaximumGPU.end();


    g_computeMaximumCPU.begin();
    // static char static_charbuf[512]{0};
    // if(g_mousePressed && awc2isMouseButtonPressed(AWC2_MOUSEBUTTON_RIGHT)) {
    //     for(i32 i = 0; i < g_reductionBufferLength; ++i) {
    //         mappedptr[i].to_strbuf(&static_charbuf[0], 512);
    //         printf("  %u => %s\n", i, static_charbuf);
    //     }
    // }

#ifdef __AVX__
    g_maxVelocity = vec2f{0.0f};
    __m256 a, b, c, eight_float_wide;
    __m128 lower, higher, four_float_wide;

    const i64 reduct_buf_m256_length = g_reductionBufferLength * sizeof(vec4f) / sizeof(__m256);
    auto* mappedptr = __rcast(f32*, g_reductionMaxMappedBuf);
    eight_float_wide = _mm256_loadu_ps(__rcast(f32*, &mappedptr[0]));
    for(i64 i = 1; i < reduct_buf_m256_length; i += 2) {
        a = _mm256_loadu_ps(&mappedptr[i]);
        b = _mm256_loadu_ps(&mappedptr[i + 1]);
        c = compareBiggerThanXY_M256(a, b);
        eight_float_wide = compareBiggerThanXY_M256(c, eight_float_wide);
    }
    /* split eight_float_wide to 2-vec4f's, store whichever's bigger */
    lower  = _mm256_extractf128_ps(eight_float_wide, false);
    higher = _mm256_extractf128_ps(eight_float_wide, true);
    four_float_wide = compareBiggerThanXY_M128(lower, higher);
    
    /* Same thing but with 2-vec2f's */
    lower  = _mm_shuffle_ps(four_float_wide, four_float_wide, _MM_SHUFFLE(0, 1, 0, 0));
    higher = _mm_shuffle_ps(four_float_wide, four_float_wide, _MM_SHUFFLE(2, 3, 0, 0));
    four_float_wide = compareBiggerThanXY_M128(lower, higher);

    /* Final Result */
    _mm_storel_pi(&g_maxVelocity.mmx, four_float_wide);

#elif defined __SSE4_1__
    g_maxVelocity = vec2f{0.0f};
    auto* mappedptr = __rcast(vec4f*, g_reductionMaxMappedBuf);
    __m128 currmax   = _mg_loadl_pi(_mg_setzero_ps(), &g_maxVelocity.mmx);
    for(i32 i = 0; i < g_reductionBufferLength; ++i) {
        __m128 x = _mg_load_ps(&mappedptr[i].x);
        currmax = compareBiggerThanXY_M128(x, currmax);
    }
    _mg_storel_pi(&g_maxVelocity.mmx, currmax);
#endif
    g_computeMaximumCPU.end();
    return;
}




static void compute_error(
    u32 previousVelocity,
    u32 currentVelocity
) {
    auto jamAFenceToWaitForGPUBufferVisibility = []() {
        if(g_fence) {
            gl::glDeleteSync(g_fence);
        }
        g_fence = gl::glFenceSync(gl::GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        return;
    };


    g_computeErrorGPU.begin();
    gr_computeErrorTexture.bind();
    gr_computeErrorTexture.uniform1i("oldFrame",     0);
    gr_computeErrorTexture.uniform1i("newFrame",     1);
    gr_computeErrorTexture.uniform1i("errorTexture", 2);
    gl::glBindTextureUnit(0, previousVelocity);
    gl::glBindTextureUnit(1, currentVelocity);
    gl::glBindImageTexture(2, gr_errorTexture, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    invokeCompute();


    gr_computeErrorEstimates.bind();
    gr_computeErrorEstimates.uniform1i("errorTexture", 0);
    gr_computeErrorEstimates.StorageBlock("reductionBuffer", 1);
    gr_computeErrorEstimates.uniform1i("ku_valuesToFetch", g_reductionFactor);
    gl::glBindTextureUnit(0, gr_errorTexture);
    gl::glBindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, 1, g_reductionErrBuffer);
    gl::glDispatchCompute(g_reductionBufferLength, 1, 1);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);

    jamAFenceToWaitForGPUBufferVisibility();
    g_computeErrorGPU.end();


    g_computeErrorCPU.begin();
    util::__memcpy(&g_prevErrorValues[0], &g_currErrorValues[0], 3);
    g_currErrorValues[0] = vec4f{1000};
    g_currErrorValues[1] = vec4f{0.0f};
    g_currErrorValues[2] = vec4f{0.0f};

    auto* mappedptr = __rcast(vec4f*, g_reductionErrMappedBuf);
    vec4f local_minmaxavg[3];
    for(i32 i = 0; i < g_reductionBufferLength; ++i) {
        util::__memcpy(&local_minmaxavg[0], &mappedptr[3 * i], 3);

        g_currErrorValues[0] = compareLessThanXY_M128(
            local_minmaxavg[0].xmm, 
            g_currErrorValues[0].xmm
        );
        g_currErrorValues[1] = compareBiggerThanXY_M128(
            local_minmaxavg[1].xmm, 
            g_currErrorValues[1].xmm
        );
        g_currErrorValues[2] += local_minmaxavg[2];
    }
    g_currErrorValues[2] *= g_invdims.x * g_invdims.y;


    f32 delta_avg_n   = vec2f{g_currErrorValues[2].x, g_currErrorValues[2].y}.length();
    f32 delta_avg_nm1 = vec2f{g_prevErrorValues[2].x, g_prevErrorValues[2].y}.length();
    g_maxSpectralRadius = delta_avg_n / delta_avg_nm1;
    g_iterationErrorN   = delta_avg_n / (g_maxSpectralRadius - 1);
    g_computeErrorCPU.end();
    return;
}


void compute_postprocess()
{
    f32 unitCoords[2]{ g_unitLength, g_unitLength };
    u32 texToRender;
    switch(__scast(DrawTarget, g_chooseTextureToRender)) {
        case DrawTarget::DYE:
        texToRender = gr_nextIterDye;
        break;
        case DrawTarget::VELOCITY_PRESSURE:
        texToRender = gr_nextIterVel;
        break;
        case DrawTarget::CURL:
        texToRender = gr_nextIterVel;
        break;
        case DrawTarget::ABS_CURL:
        texToRender = gr_nextIterVel;
        break;
        case DrawTarget::ERROR_VELOCITY:
        texToRender = gr_errorTexture;
        break;
        case DrawTarget::ERROR_PRESSURE:
        texToRender = gr_errorTexture;
        break;
        case DrawTarget::BOUNDARY:
        texToRender = DEFAULT32;
        break;
        case DrawTarget::VELOCITY_X:
        texToRender = gr_nextIterVel;
        break;
        case DrawTarget::VELOCITY_Y:
        texToRender = gr_nextIterVel;
        break;
        case DrawTarget::PREESURE:
        texToRender = gr_nextIterVel;
        break;
        case DrawTarget::CFL_CONTOUR:
        texToRender = gr_nextIterVel;
        break;
        default:
        texToRender = DEFAULT32;
        break;
    }
    gr_computeRenderToTex.bind();
    gr_computeRenderToTex.uniform1i("fieldSampler",  0);
    gr_computeRenderToTex.uniform1i("screentexture", 1);
    gr_computeRenderToTex.uniform1ui("ku_selectTextureDraw", __scast(u32, g_chooseTextureToRender));
    gr_computeRenderToTex.uniform1ui("ku_selectColourMap",   __scast(u32, g_chooseColourMap));
    gr_computeRenderToTex.uniform1f("ku_brightness",         g_textureHighlightSmallValue);
    gr_computeRenderToTex.uniform1f("ku_dt",                 g_normdt);
    gr_computeRenderToTex.uniform2fv("ku_colormapMinmax",    g_colorTableMinMax.begin());
    gr_computeRenderToTex.uniform2iv("ku_simdims",           g_dims.begin());
    gr_computeRenderToTex.uniform2fv("ku_simUnitCoord",      &unitCoords[0]);
    gl::glBindTextureUnit(0, texToRender);
    gl::glBindImageTexture(1, gr_drawTexture, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    invokeCompute();
    return;
}


static void invokeCompute(gl::MemoryBarrierMask mask)
{
    gl::glDispatchCompute(
        g_computeInvocationSize[0], 
        g_computeInvocationSize[1], 
        g_computeInvocationSize[2]
    );
    gl::glMemoryBarrier(mask);
    return;
}


u32 addForces(FillType type, u32 interactTexIn)
{
    f32 unitCoords[2]{ g_unitLength, g_unitLength };
    auto winsize = awc2getCurrentContextViewport();
    vec2i winsizei32 = vec2i{__scast(i32, winsize.x), __scast(i32, winsize.y)};
    u32 interactTexOut = gr_outTexShader0;
    bool first = true;

    
    gr_computeInteractive.bind();
    gr_computeInteractive.uniform1i("prevFrame", 0);
    gr_computeInteractive.uniform1i("nextFrame", 1);
    gr_computeInteractive.uniform2iv("ku_simdims",        g_dims.begin());
    gr_computeInteractive.uniform2iv("ku_windims",        &winsizei32.x);
    gr_computeInteractive.uniform1f("ku_dt",              g_normdt);
    gr_computeInteractive.uniform1f("ku_vorticityFactor", g_confineVorticity);
    gr_computeInteractive.uniform2fv("ku_simUnitCoord",   &unitCoords[0]);
    for(auto& source : g_sources) 
    {
        /* 
            it might be that we're using force_and_dye, which needs to be processed
            both by force, and both by dye.
        */
        bool extra_cond = source.m_type == FillType::FORCE_AND_DYE
            && (type == FillType::FORCE || type == FillType::DYE);


        if(!source.m_enabled || (source.m_type != type && !extra_cond))
            continue;


        vec4f choose_color = type == FillType::FORCE ? source.m_force : source.m_color;
        gr_computeInteractive.uniform2fv("ku_sourcePosition", source.m_position.begin());
        gr_computeInteractive.uniform4fv("ku_sourceValue",    choose_color.begin());
        gr_computeInteractive.uniform1f("ku_sourceRadius",    source.m_radius);
        gr_computeInteractive.uniform1ui("ku_enabled",        source.m_enabled);
        gr_computeInteractive.uniform1ui("ku_interactType",   __scast(u32, type));
        gl::glBindTextureUnit(0, interactTexIn);
        gl::glBindImageTexture(1, interactTexOut, 0, false, 0, 
            gl::GL_WRITE_ONLY,
            gl::GL_RGBA32F
        );
        invokeCompute(gl::GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


        if(first) { 
            /* 
                We need to preserve gr_prevIterVel for the advection step, 
                So we can't ping-pong between gr_prevIterVel & gr_outTexShader0
            */
            interactTexIn = gr_tmpTexture1;
            first = false;
        }
        std::swap(interactTexIn, interactTexOut);
    }


    return interactTexIn;
}


u32 advect(u32 previousStep, bool keepOnlyVelocity, u32 outTex)
{
    f32 unitCoords[2]{ g_unitLength, g_unitLength };

    gr_computeAdvection.bind();
    gr_computeAdvection.uniform1i("quantityField", 0);
    gr_computeAdvection.uniform1i("velocityField", 1);
    gr_computeAdvection.uniform1i("outputField",   2);
    gr_computeAdvection.uniform1ui("ku_writeAllComponents", !keepOnlyVelocity);
    gr_computeAdvection.uniform1f("ku_dt",                  g_normdt);
    gr_computeAdvection.uniform2fv("ku_simUnitCoord",       &unitCoords[0]);
    gr_computeAdvection.uniform2iv("ku_simdims",            g_dims.begin());
    gl::glBindTextureUnit(0, previousStep);
    gl::glBindTextureUnit(1, gr_prevIterVel);
    gl::glBindImageTexture(2, outTex, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    invokeCompute(gl::GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


    return outTex;
}


u32 diffuseVelocity(u32 previousStep)
{
    f32 unitCoords[2]{ g_unitLength, g_unitLength };
    u32 textureInput  = previousStep;
    u32 textureOutput = gr_tmpTexture0;
    gr_computeDiffusionVel.bind();
    gr_computeDiffusionVel.uniform1i("velocityIn",  0);
    gr_computeDiffusionVel.uniform1i("velocityOut", 1);
    gr_computeDiffusionVel.uniform1f("ku_dt",        g_dt);
    gr_computeDiffusionVel.uniform1f("ku_viscosity", g_kinematicViscosity);
    gr_computeDiffusionVel.uniform2iv("ku_simdims",      g_dims.begin());
    gr_computeDiffusionVel.uniform2fv("ku_simUnitCoord", &unitCoords[0]);
    gl::glBindTextureUnit(0, textureInput);
    gl::glBindImageTexture(1, textureOutput, 0, 0, 0, gl::GL_WRITE_ONLY, gl::GL_RGBA32F);
    invokeCompute(gl::GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


    i32 i = 0;
    textureInput  = gr_tmpTexture0;
    textureOutput = gr_outTexShader2;
    while(i < g_maximumJacobiIterations) {
        gl::glBindTextureUnit(0, textureInput);
        gl::glBindImageTexture(1, textureOutput, 0, 0, 0, gl::GL_WRITE_ONLY, gl::GL_RGBA32F);
        invokeCompute(gl::GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        std::swap(textureInput, textureOutput);
        ++i;
    }


    return textureInput;
}


u32 diffusePressure(u32 previousStep)
{
    f32 unitCoords[2]{ g_unitLength, g_unitLength };

    gr_computeDivergence.bind();
    gr_computeDivergence.uniform1i("intermediateVelocityField", 0);
    gr_computeDivergence.uniform1i("oldPressureFieldValue",     1);
    gr_computeDivergence.uniform1i("outputField",               2);
    gr_computeDivergence.uniform2iv("ku_simdims",               g_dims.begin());
    gr_computeDivergence.uniform2fv("ku_simUnitCoord", &unitCoords[0]);
    gl::glBindTextureUnit(0, previousStep);
    gl::glBindTextureUnit(1, gr_prevIterVel);
    gl::glBindImageTexture(2, gr_outTexShader3, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    invokeCompute(gl::GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


    u32 textureInput  = gr_outTexShader3;
    u32 textureOutput = gr_tmpTexture0;
    gr_computeDiffusionPressure.bind();
    gr_computeDiffusionPressure.uniform1i("pressureIn",  0);
    gr_computeDiffusionPressure.uniform1i("pressureOut", 1);
    gr_computeDiffusionPressure.uniform2iv("ku_simdims",       g_dims.begin());
    gr_computeDiffusionPressure.uniform2fv("ku_simUnitCoord",  &unitCoords[0]);
    gr_computeDiffusionPressure.uniform1f("ku_inverseDensity", 1.0f / g_densityPressure);
    gl::glBindTextureUnit(0, textureInput);
    gl::glBindImageTexture(1, textureOutput, 0, 0, 0, gl::GL_WRITE_ONLY, gl::GL_RGBA32F);
    invokeCompute(gl::GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


    i32 i = 0;
    textureInput  = gr_tmpTexture0;
    textureOutput = gr_outTexShader4;
    while(i < g_maximumJacobiIterations) {
        gl::glBindTextureUnit(0, textureInput);
        gl::glBindImageTexture(1, textureOutput, 0, 0, 0, gl::GL_WRITE_ONLY, gl::GL_RGBA32F);
        invokeCompute(gl::GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        std::swap(textureInput, textureOutput);
        ++i;
    }


    return textureInput;
}


void updateVelocities(u32 newvel, u32 newpressure)
{
    f32 unitCoords[2]{ g_unitLength, g_unitLength };

    /* add together all the shit we computed for the next iteration */
    gr_computeNewVelocity.bind();
    gr_computeNewVelocity.uniform1i("intermediateVelocity", 0);
    gr_computeNewVelocity.uniform1i("intermediatePressure", 1);
    gr_computeNewVelocity.uniform1i("updatedFields",        2);
    gr_computeNewVelocity.uniform2iv("ku_simdims",      g_dims.begin());
    gr_computeNewVelocity.uniform2fv("ku_simUnitCoord", &unitCoords[0]);
    gl::glBindTextureUnit(0, newvel);
    gl::glBindTextureUnit(1, newpressure);
    gl::glBindImageTexture(2, gr_nextIterVel, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    invokeCompute(gl::GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    return;
}