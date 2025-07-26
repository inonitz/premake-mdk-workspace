#include "backend17.hpp"
#include <awc2/C/awc2.h>
#include <util2/C/marker4.h>
#include "vars.hpp"


using namespace optimize17;


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



u32 optimize17::compute_fluid()
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
    UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(g_computeCFLTime, compute_cfl(nextIterationVel));

    return nextIterationVel;
}


static void compute_velocity(u32 previousIteration, u32 nextIteration)
{
    bool userInteractionFlag = g_mousePressed && awc2isMouseButtonPressed(AWC2_MOUSEBUTTON_RIGHT);
    vec3u computeInvocationSize = { g_dims.x / 64, g_dims.y, 1 };

    if(likely(!userInteractionFlag)) {
        gl::glClearTexImage(gr_outTexShader0, 0, gl::GL_RGBA, gl::GL_FLOAT, nullptr);
    } else {
        gr_computeInteractive.bind();
        gr_computeInteractive.uniform1i("pong", 0);
        gr_computeInteractive.uniform1f("ku_dt",       g_dt);
        gr_computeInteractive.uniform2iv("ku_simdims", g_dims.begin());
        gr_computeInteractive.uniform2iv("ku_windims", g_windims.begin());
        gr_computeInteractive.uniform2fv("ku_mouseDragForce", g_mouseDragForce.begin()   );
        gr_computeInteractive.uniform2fv("ku_mouseDragPos",   g_mouseDragPosition.begin());
        gr_computeInteractive.uniform1f("ku_splatterRadius",  g_splatterRadius); 
        gr_computeInteractive.uniform4fv("ku_splatterColor",  g_splatterColor.begin());
        gr_computeInteractive.uniform1ui("ku_mousePressed",   g_mousePressed);
        gl::glBindImageTexture(0, gr_outTexShader0, 0, false, 0, 
            gl::GL_WRITE_ONLY,
            gl::GL_RGBA32F
        );
        gl::glDispatchCompute(computeInvocationSize.x, computeInvocationSize.y, computeInvocationSize.z);
    }


    gr_computeAdvection.bind();
    gr_computeAdvection.uniform1i("quantityField", 0);
    gr_computeAdvection.uniform1i("velocityField", 1);
    gr_computeAdvection.uniform1i("outputField",   2);
    gr_computeAdvection.uniform1f("ku_dt",            g_dt);
    gr_computeAdvection.uniform2iv("ku_simdims",      g_dims.begin());
    gr_computeAdvection.uniform2fv("ku_simUnitCoord", g_simUnitCoords.begin());
    gl::glBindTextureUnit(0, previousIteration);
    gl::glBindTextureUnit(1, previousIteration);
    gl::glBindImageTexture(1, gr_outTexShader1, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    gl::glDispatchCompute(computeInvocationSize.x, computeInvocationSize.y, computeInvocationSize.z);


    /* Compute diffuse component of the velocity field to diffusion texture (gr_outTexShader2) */
    u32 textureInput, textureOutput;

    gr_computeDiffusionVel.bind();
    gr_computeDiffusionVel.uniform1i("initialField", 0);
    gr_computeDiffusionVel.uniform1i("outputField",  1);
    gr_computeDiffusionVel.uniform1i("ku_valuesToFetch", g_reductionFactor);
    gr_computeDiffusionVel.uniform1f("ku_dt",            g_dt);
    gr_computeDiffusionVel.uniform1f("ku_viscosity",     g_viscosity);
    gr_computeDiffusionVel.uniform2iv("ku_simdims",      g_dims.begin());
    gr_computeDiffusionVel.uniform2fv("ku_simUnitCoord", g_simUnitCoords.begin());
    gl::glBindTextureUnit(0, previousIteration);
    gl::glBindImageTexture(1, gr_outTexShader2, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    gl::glDispatchCompute(computeInvocationSize.x, computeInvocationSize.y, computeInvocationSize.z);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);


    textureInput  = gr_outTexShader2;
    textureOutput = gr_tmpTexture0;
    for(i32 i = 1; i < g_maximumJacobiIterations; ++i) {
        gl::glBindTextureUnit(0, textureInput);
        gl::glBindImageTexture(1, textureOutput, 0, false, 0, 
            gl::GL_WRITE_ONLY, 
            gl::GL_RGBA32F
        );
        gl::glDispatchCompute(computeInvocationSize.x, computeInvocationSize.y, computeInvocationSize.z);


        u32 tmp = textureInput;
        textureInput  = textureOutput;
        textureOutput = tmp;
        gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
    }
    gl::glFlush();

    /* add all the shit we computed together for the next iteration */
    gr_computeAdd.bind();
    gr_computeAdd.uniform1i("externuserField", 0);
    gr_computeAdd.uniform1i("advectedField",   1);
    gr_computeAdd.uniform1i("diffusedField",   2);
    gr_computeAdd.uniform1i("outputField",     3);
    gl::glBindTextureUnit(0, gr_outTexShader0);
    gl::glBindTextureUnit(1, gr_outTexShader1);
    gl::glBindTextureUnit(2, gr_outTexShader2);
    gl::glBindImageTexture(3, gr_outTexShader3, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    gl::glDispatchCompute(computeInvocationSize.x, computeInvocationSize.y, computeInvocationSize.z);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);


    gr_computeDivergence.bind();
    gr_computeDivergence.uniform1i("intermediateVelocityField", 0);
    gr_computeDivergence.uniform1i("oldPressureFieldValue",     1);
    gr_computeDivergence.uniform1i("outputField",               2);
    gr_computeDivergence.uniform2fv("ku_simUnitCoord", g_simUnitCoords.begin());
    gl::glBindTextureUnit(0, gr_outTexShader3);
    gl::glBindTextureUnit(1, previousIteration);
    gl::glBindImageTexture(2, gr_outTexShader4, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    gl::glDispatchCompute(computeInvocationSize.x, computeInvocationSize.y, computeInvocationSize.z);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);



    /* Compute the new pressure to .z component (gr_outTexShader5) */
    gr_computeDiffusionPressure.bind();
    gr_computeDiffusionPressure.uniform1i("oldPressureField", 0);
    gr_computeDiffusionPressure.uniform1i("newPressureField", 1);
    gr_computeDiffusionPressure.uniform2iv("ku_simdims", g_dims.begin());
    gr_computeDiffusionPressure.uniform2fv("ku_simUnitCoord", g_simUnitCoords.begin());

    gl::glBindTextureUnit(0, gr_outTexShader4);
    gl::glBindImageTexture(1, gr_outTexShader5, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    gl::glDispatchCompute(computeInvocationSize.x, computeInvocationSize.y, computeInvocationSize.z);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);


    textureInput  = gr_outTexShader5;
    textureOutput = gr_tmpTexture0;
    for(i32 i = 1; i < g_maximumJacobiIterations; ++i) {
        gl::glBindTextureUnit(0, textureInput);
        gl::glBindImageTexture(1, textureOutput, 0, false, 0, 
            gl::GL_WRITE_ONLY, 
            gl::GL_RGBA32F
        );
        gl::glDispatchCompute(computeInvocationSize.x, computeInvocationSize.y, computeInvocationSize.z);


        u32 tmp = textureInput;
        textureInput  = textureOutput;
        textureOutput = tmp;
        gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
    }
    gl::glFlush();
    // gl::glBindTextureUnit(0, textureInput);
    // gl::glBindImageTexture(1, gr_outTexShader5, 0, false, 0, 
    //     gl::GL_WRITE_ONLY, 
    //     gl::GL_RGBA32F
    // );
    // gl::glDispatchCompute(g_dims.x, g_dims.y, 1);
    // gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);


    /* add together all the shit we computed for the next iteration */
    gr_computeNewVelocity.bind();
    gr_computeNewVelocity.uniform1i("intermediate",    0);
    gr_computeNewVelocity.uniform1i("updatedFields",   1);
    gr_computeNewVelocity.uniform2fv("ku_simUnitCoord", g_simUnitCoords.begin());
    gl::glBindTextureUnit(0, gr_outTexShader5);
    gl::glBindImageTexture(1, nextIteration, 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    gl::glDispatchCompute(computeInvocationSize.x, computeInvocationSize.y, computeInvocationSize.z);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
    return;
}


static void compute_dye(
    notused u32 velocityPressureField,
    notused u32 previousIterationDye, 
    notused u32 nextIterationDye
) {
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
    gl::glBindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, 1, g_reductionMaxTexture);
    gl::glBindTextureUnit(0, texture);
    gr_computeCFLCondition.bind();
    gr_computeCFLCondition.uniform1i("originalTexture", 0);
    gr_computeCFLCondition.StorageBlock("reductionMaximumBuffer", 1);
    gr_computeCFLCondition.uniform1i("ku_valuesToFetch", g_reductionFactor);
    gl::glDispatchCompute(g_reductionBufferLength, 1, 1);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
    

    jamAFenceToWaitForGPUBufferVisibility();


    g_retrieveTextureData.begin();
    // static char static_charbuf[512]{0};
    // if(g_mousePressed && awc2isMouseButtonPressed(AWC2_MOUSEBUTTON_RIGHT)) {
    //     for(i32 i = 0; i < g_reductionBufferLength; ++i) {
    //         mappedptr[i].to_strbuf(&static_charbuf[0], 512);
    //         printf("  %u => %s\n", i, static_charbuf);
    //     }
    // }

    bool biggerX, biggerY;
    auto* mappedptr = __rcast(vec4f*, g_reductionMaxMappedBuf);
    for(i32 i = 0; i < g_reductionBufferLength; ++i) {
        biggerX = boolean(mappedptr[i].x > g_velocityCFL.x);
        biggerY = boolean(mappedptr[i].y > g_velocityCFL.y);
        g_velocityCFL.x = __scast(f32, biggerX) * mappedptr[i].x + __scast(f32, !biggerX) * g_velocityCFL.x;
        g_velocityCFL.y = __scast(f32, biggerY) * mappedptr[i].y + __scast(f32, !biggerY) * g_velocityCFL.y;
    }
    g_retrieveTextureData.end();
    return;
}