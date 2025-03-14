#include "awc2/context.hpp"
#include <awc2/awc2.hpp>
#include <glbinding/gl/gl.h>


static void fbo_callback(__unused AWC2::user_callback_winsize_struct const* winsize) {
    return;
}
static void key_callback(AWC2::user_callback_keyboard_struct const* key) {    
    if(key->keyStroke == AWC2::Input::keyCode::ESCAPE && key->action == AWC2::Input::inputState::PRESS) {
        AWC2::closeContext(key->context_id);
    }
    return;
}



int awc2_context_management_with_bitmap()
{
    AWC2::init();
    
    
    __unused u32 framecount{0};
    f32 framecountf{0};
    AWC2::ContextMap contexts;
    AWC2::ContextID context_id[3] = {
        AWC2::createContext(),
        AWC2::createContext(),
        AWC2::createContext()
    };
    AWC2::initializeContext(context_id[0], 640, 480, AWC2::WindowDescriptor{});
    AWC2::initializeContext(context_id[1], 640, 480, AWC2::WindowDescriptor{});
    AWC2::initializeContext(context_id[2], 640, 480, AWC2::WindowDescriptor{});
    contexts = AWC2::getActiveContextBitmap();
    u8 i = 1;
    while(contexts != 0) {
        AWC2::setContextUserCallback(i, &fbo_callback);
        AWC2::setContextUserCallback(i, &key_callback);
        contexts >>= 1;
    }


    i = 1;
    contexts = AWC2::getActiveContextBitmap();
    while(contexts != 0)
    {
        AWC2::newFrame();
        while(contexts != 0) {
            ++framecountf;
            AWC2::setCurrentContext(i);
            AWC2::begin();
            gl::glClearColor(1.0f, 0.0f, framecountf / 255.0f, 0.0f);
            gl::glClear(gl::ClearBufferMask{gl::GL_COLOR_BUFFER_BIT});
            AWC2::end();
            contexts >>= 1;
            ++i;
        }
        i           = 1;
        contexts    = AWC2::getActiveContextBitmap();
        framecountf = (framecountf > 255.0f) ? 0.0f : framecountf;
    }


    for(u8 i = 0; i < __carraysize(context_id); ++i) {
        AWC2::destroyContext(context_id[i]);
    }
    AWC2::destroy();
    return 0;
}