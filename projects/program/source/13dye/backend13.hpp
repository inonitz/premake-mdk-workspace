#pragma once
#include <util2/C/base_type.h>
#include <util2/time.hpp>


namespace program4 {

u8               getContextID();
util2::Time::Timestamp& getFrameTime();
bool             getSlowRenderFlag();
void initializeLibrary();
void destroyLibrary();
void initializeGraphics();
void destroyGraphics();
void render();

}