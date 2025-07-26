#include "backend24.hpp"
#include <awc2/C/awc2.h>
#include <util2/C/marker4.h>
#include <immintrin.h>
#include "vars.hpp"


using namespace morebutton24;


static void compute_velocity(
    u32 previousIteration, 
    u32 nextIteration
);
static void compute_dye(
    u32 velocityPressureField, 
    u32 previousIteration,
    u32 nextIteration
);
static void compute_cfl(u32 texture);
static void compute_cfl_new(u32 texture);
static void compute_error(
    u32 previousVelocity,
    u32 currentVelocity,
    u32 errorTex
);


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


u32 morebutton24::compute_fluid()
{
    u32 previousIterationVel, nextIterationVel;
    u32 previousIterationDye, nextIterationDye;
    if(g_frameCounter % 2) {
        previousIterationVel = gr_simTexture1;
        nextIterationVel     = gr_simTexture0;

        previousIterationDye = gr_dyeTexture1;
        nextIterationDye     = gr_dyeTexture0;
    } else {
        previousIterationVel = gr_simTexture0;
        nextIterationVel     = gr_simTexture1;

        previousIterationDye = gr_dyeTexture0;
        nextIterationDye     = gr_dyeTexture1;
    }


    UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(g_computeVelTime, compute_velocity(previousIterationVel, nextIterationVel));
    UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(g_computeDyeTime, compute_dye(nextIterationVel, previousIterationDye, nextIterationDye));
    UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(g_computeCFLTime, compute_cfl_new(nextIterationVel));
    UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(g_computeErrEstimateTime, compute_error(
        previousIterationVel,
        nextIterationVel,
        gr_outTexShader9
    ));


    u32 resultTex = 0;
    switch(g_chooseTextureToRender) {
        case TEXTURE_DRAW_VELOCITY_PRESSURE: 
        resultTex = nextIterationVel; 
        break;
        case TEXTURE_DRAW_DYE: 
        resultTex = nextIterationDye; 
        break;
        case TEXTURE_DRAW_CURL:
        resultTex = nextIterationVel; 
        break; 
        case TEXTURE_DRAW_ABS_CURL:
        resultTex = nextIterationVel; 
        break;
        case TEXTURE_DRAW_ERROR_VELOCITY:
        resultTex = gr_outTexShader9;
        break;
        case TEXTURE_DRAW_ERROR_PRESSURE:
        resultTex = gr_outTexShader9;
        break;
        default: 
        break;
    }
    return resultTex;
}


static void compute_velocity(u32 previousIteration, u32 nextIteration)
{
    bool chooseDrawType = (
        g_chooseUserDrawFillType == USER_DRAW_FILL_TYPE_FORCE || 
        g_chooseUserDrawFillType == USER_DRAW_FILL_TYPE_FORCE_AND_DYE
    );
    if(chooseDrawType) {
        gr_computeInteractive.bind();
        gr_computeInteractive.uniform1i("prevFrame", 0);
        gr_computeInteractive.uniform1i("nextFrame",  1);
        gr_computeInteractive.uniform2iv("ku_simdims",        g_dims.begin());
        gr_computeInteractive.uniform2iv("ku_windims",        g_windowSize.begin());
        gr_computeInteractive.uniform1f("ku_dt",              g_normdt);
        gr_computeInteractive.uniform1f("ku_vorticityFactor", g_confineVorticity);
        gr_computeInteractive.uniform1f("ku_splatterRadius",  g_splatterRadius);
        gr_computeInteractive.uniform2fv("ku_simUnitCoord",   g_simUnitCoords.begin());
        gr_computeInteractive.uniform2fv("ku_mouseDragPos",   g_mousePosition.begin());
        gr_computeInteractive.uniform4fv("ku_splatterValue",  g_mousedxdy.begin());
        gr_computeInteractive.uniform1ui("ku_mousePressed",   g_mousePressOverride);
        gr_computeInteractive.uniform1ui("ku_chooseInteractionType", USER_DRAW_FILL_TYPE_FORCE);
        gr_computeInteractive.uniform1ui("ku_useVorticity", g_useVorticityConfinement);
        gl::glBindTextureUnit(0, previousIteration);
        gl::glBindImageTexture(1, gr_outTexShader0, 0, false, 0, 
            gl::GL_WRITE_ONLY,
            gl::GL_RGBA32F
        );
        gl::glDispatchCompute(g_computeInvocationSize.x, g_computeInvocationSize.y, g_computeInvocationSize.z);
        gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
    }


    gr_computeAdvection.bind();
    gr_computeAdvection.uniform1i("quantityField", 0);
    gr_computeAdvection.uniform1i("velocityField", 1);
    gr_computeAdvection.uniform1i("outputField",   2);
    gr_computeAdvection.uniform1ui("ku_writeAllComponents", false);
    gr_computeAdvection.uniform1f("ku_dt",                  g_normdt);
    gr_computeAdvection.uniform2fv("ku_simUnitCoord",       g_simUnitCoords.begin());
    gr_computeAdvection.uniform2iv("ku_simdims",            g_dims.begin());
    gl::glBindTextureUnit(0, chooseDrawType ? gr_outTexShader0 : previousIteration);
    gl::glBindTextureUnit(1, previousIteration);
    gl::glBindImageTexture(2, gr_outTexShader1, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    gl::glDispatchCompute(g_computeInvocationSize.x, g_computeInvocationSize.y, g_computeInvocationSize.z);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);


    /* Compute diffuse component of the velocity field to diffusion texture (gr_outTexShader2) */
    u32 textureInput, textureOutput;
    gr_computeDiffusionVel.bind();
    gr_computeDiffusionVel.uniform1i("initialField", 0);
    gr_computeDiffusionVel.uniform1i("outputField",  1);
    gr_computeDiffusionVel.uniform1f("ku_dt",            g_normdt);
    gr_computeDiffusionVel.uniform1f("ku_viscosity",     g_kinematicViscosity);
    gr_computeDiffusionVel.uniform2iv("ku_simdims",      g_dims.begin());
    gr_computeDiffusionVel.uniform2fv("ku_simUnitCoord", g_simUnitCoords.begin());
    gl::glBindTextureUnit(0, gr_outTexShader1);
    gl::glBindImageTexture(1, gr_outTexShader2, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    gl::glDispatchCompute(g_computeInvocationSize.x, g_computeInvocationSize.y, g_computeInvocationSize.z);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);


    textureInput  = gr_outTexShader2;
    textureOutput = gr_tmpTexture0;
    for(i32 i = 1; i < g_maximumJacobiIterations; ++i) {
        gl::glBindTextureUnit(0, textureInput);
        gl::glBindImageTexture(1, textureOutput, 0, false, 0, 
            gl::GL_WRITE_ONLY, 
            gl::GL_RGBA32F
        );
        gl::glDispatchCompute(g_computeInvocationSize.x, g_computeInvocationSize.y, g_computeInvocationSize.z);


        u32 tmp = textureInput;
        textureInput  = textureOutput;
        textureOutput = tmp;
        gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
    }


    gr_computeDivergence.bind();
    gr_computeDivergence.uniform1i("intermediateVelocityField", 0);
    gr_computeDivergence.uniform1i("oldPressureFieldValue",     1);
    gr_computeDivergence.uniform1i("outputField",               2);
    gr_computeDivergence.uniform2fv("ku_simUnitCoord", g_simUnitCoords.begin());
    gl::glBindTextureUnit(0, gr_outTexShader2);
    gl::glBindTextureUnit(1, previousIteration);
    gl::glBindImageTexture(2, gr_outTexShader3, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    gl::glDispatchCompute(g_computeInvocationSize.x, g_computeInvocationSize.y, g_computeInvocationSize.z);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);



    /* Compute the new pressure to .z component (gr_outTexShader5) */
    gr_computeDiffusionPressure.bind();
    gr_computeDiffusionPressure.uniform1i("oldPressureField", 0);
    gr_computeDiffusionPressure.uniform1i("newPressureField", 1);
    gr_computeDiffusionPressure.uniform2iv("ku_simdims", g_dims.begin());
    gr_computeDiffusionPressure.uniform2fv("ku_simUnitCoord", g_simUnitCoords.begin());
    gr_computeDiffusionPressure.uniform1f("ku_inverseDensity", 1.0f / g_densityPressure);

    gl::glBindTextureUnit(0, gr_outTexShader3);
    gl::glBindImageTexture(1, gr_outTexShader4, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    gl::glDispatchCompute(g_computeInvocationSize.x, g_computeInvocationSize.y, g_computeInvocationSize.z);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);


    textureInput  = gr_outTexShader4;
    textureOutput = gr_tmpTexture0;
    for(i32 i = 1; i < g_maximumJacobiIterations; ++i) {
        gl::glBindTextureUnit(0, textureInput);
        gl::glBindImageTexture(1, textureOutput, 0, false, 0, 
            gl::GL_WRITE_ONLY, 
            gl::GL_RGBA32F
        );
        gl::glDispatchCompute(g_computeInvocationSize.x, g_computeInvocationSize.y, g_computeInvocationSize.z);


        u32 tmp = textureInput;
        textureInput  = textureOutput;
        textureOutput = tmp;
        gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
    }


    /* add together all the shit we computed for the next iteration */
    gr_computeNewVelocity.bind();
    gr_computeNewVelocity.uniform1i("intermediate",    0);
    gr_computeNewVelocity.uniform1i("updatedFields",   1);
    gr_computeNewVelocity.uniform2fv("ku_simUnitCoord", g_simUnitCoords.begin());
    gl::glBindTextureUnit(0, gr_outTexShader4);
    gl::glBindImageTexture(1, nextIteration, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    gl::glDispatchCompute(g_computeInvocationSize.x, g_computeInvocationSize.y, g_computeInvocationSize.z);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
    return;
}


static void compute_dye(
    u32 velocityPressureField,
    u32 previousIterationDye, 
    u32 nextIterationDye
) {
    bool chooseDrawType = (
        g_chooseUserDrawFillType == USER_DRAW_FILL_TYPE_DYE || 
        g_chooseUserDrawFillType == USER_DRAW_FILL_TYPE_FORCE_AND_DYE
    );
    /* 
        Little bug that I don't really care to fix:
        Vorticity is added only while we're inputting a force (chooseDrawType...).
        In reality, if the user wants vorticity,
        it should be added no matter what the input method is.
        The way to fix this would be to have a forces/vorticity pass,
        that isn't affected by the chooseDrawType conditional. e.g =>
        if(g_useVorticity) {
            gr_computeInteractive(interaction_type = nil, use_vorticity = true)
        }
        ...
    */
    if(chooseDrawType) {
        gr_computeInteractive.bind();
        gr_computeInteractive.uniform1i("prevFrame", 0);
        gr_computeInteractive.uniform1i("nextFrame",  1);
        gr_computeInteractive.uniform2iv("ku_simdims",        g_dims.begin());
        gr_computeInteractive.uniform2iv("ku_windims",        g_windowSize.begin());
        gr_computeInteractive.uniform1f("ku_dt",              g_normdt);
        gr_computeInteractive.uniform1f("ku_vorticityFactor", g_confineVorticity);
        gr_computeInteractive.uniform1f("ku_splatterRadius",  g_splatterRadius);
        gr_computeInteractive.uniform2fv("ku_simUnitCoord",   g_simUnitCoords.begin());
        gr_computeInteractive.uniform2fv("ku_mouseDragPos",   g_mousePosition.begin());
        gr_computeInteractive.uniform4fv("ku_splatterValue",  g_splatterColor.begin());
        gr_computeInteractive.uniform1ui("ku_mousePressed",   g_mousePressOverride);
        gr_computeInteractive.uniform1ui("ku_chooseInteractionType", USER_DRAW_FILL_TYPE_DYE);
        gr_computeInteractive.uniform1ui("ku_useVorticity", false);
        gl::glBindTextureUnit(0, previousIterationDye);
        gl::glBindImageTexture(1, gr_outTexShader6, 0, false, 0, 
            gl::GL_WRITE_ONLY,
            gl::GL_RGBA32F
        );
        gl::glDispatchCompute(g_computeInvocationSize.x, g_computeInvocationSize.y, g_computeInvocationSize.z);
        gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
    }


    gr_computeAdvection.bind();
    gr_computeAdvection.uniform1i("quantityField", 0);
    gr_computeAdvection.uniform1i("velocityField", 1);
    gr_computeAdvection.uniform1i("outputField",   2);
    gr_computeAdvection.uniform1ui("ku_writeAllComponents", true);
    gr_computeAdvection.uniform1f("ku_dt",                  g_normdt);
    gr_computeAdvection.uniform2fv("ku_simUnitCoord",       g_simUnitCoords.begin());
    gr_computeAdvection.uniform2iv("ku_simdims",            g_dims.begin());
    gl::glBindTextureUnit(0, chooseDrawType ? gr_outTexShader6 : previousIterationDye);
    gl::glBindTextureUnit(1, velocityPressureField);
    gl::glBindImageTexture(2, nextIterationDye, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    gl::glDispatchCompute(g_computeInvocationSize.x, g_computeInvocationSize.y, g_computeInvocationSize.z);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
    return;
}


static void compute_cfl(u32 texture)
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

    g_maxVelocity = vec2f{0.0f};
    bool biggerX, biggerY;
    auto* mappedptr = __rcast(vec4f*, g_reductionMaxMappedBuf);
    for(i32 i = 0; i < g_reductionBufferLength; ++i) {
        biggerX = boolean(mappedptr[i].x > g_maxVelocity.x);
        biggerY = boolean(mappedptr[i].y > g_maxVelocity.y);
        g_maxVelocity.x = __scast(f32, biggerX) * mappedptr[i].x + __scast(f32, !biggerX) * g_maxVelocity.x;
        g_maxVelocity.y = __scast(f32, biggerY) * mappedptr[i].y + __scast(f32, !biggerY) * g_maxVelocity.y;
    }
    g_computeMaximumCPU.end();
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
    __m128 currmax   = _mm_loadl_pi(_mm_setzero_ps(), &g_maxVelocity.mmx);
    for(i32 i = 0; i < g_reductionBufferLength; ++i) {
        __m128 x = _mm_load_ps(&mappedptr[i].x);
        currmax = compareBiggerThanXY_M128(x, currmax);
    }
    _mm_storel_pi(&g_maxVelocity.mmx, currmax);
#endif
    g_computeMaximumCPU.end();
    return;
}



static void compute_error(
    u32 previousVelocity,
    u32 currentVelocity,
    u32 errorTex
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
    gl::glBindImageTexture(2, errorTex, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    gl::glDispatchCompute(g_computeInvocationSize.x, g_computeInvocationSize.y, g_computeInvocationSize.z);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);


    gr_computeErrorEstimates.bind();
    gr_computeErrorEstimates.uniform1i("errorTexture", 0);
    gr_computeErrorEstimates.StorageBlock("reductionBuffer", 1);
    gr_computeErrorEstimates.uniform1i("ku_valuesToFetch", g_reductionFactor);
    gl::glBindTextureUnit(0, errorTex);
    gl::glBindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, 1, g_reductionErrBuffer);
    gl::glDispatchCompute(g_reductionBufferLength, 1, 1);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);

    jamAFenceToWaitForGPUBufferVisibility();
    g_computeErrorGPU.end();


    g_computeErrorCPU.begin();
    util2::memcpy(&g_prevErrorValues[0], &g_currErrorValues[0], 3);
    g_currErrorValues[0] = vec4f{1000};
    g_currErrorValues[1] = vec4f{0.0f};
    g_currErrorValues[2] = vec4f{0.0f};

    auto* mappedptr = __rcast(vec4f*, g_reductionErrMappedBuf);
    vec4f local_minmaxavg[3];
    for(i32 i = 0; i < g_reductionBufferLength; ++i) {
        util2::memcpy(&local_minmaxavg[0], &mappedptr[3 * i], 3);

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