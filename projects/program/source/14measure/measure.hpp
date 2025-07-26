#pragma once
#include <util2/C/base_type.h>


/*
    Appearently sending ~100 commands to the gpu via opengl is a little costly,
    which I unfortunately figured out by wasting 5 hours of my life.

    unless you use glFinish()/glFlush() (which will force all previous gpu commands to execute & sync),
    execution of the gpu commands will be batched until one calls swapBuffers(),
    after which all gpu work will be finished.
    This ^ is not good, and I didn't know this
    Because there is NO way that I currently know of to perform
    compute operations asynchronously, i need to reduce the amount of calls I make.
*/
i32 started_optimization_derailed_to_measuring_everything();
