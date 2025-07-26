#pragma once
#include <util2/C/base_type.h>
#include <util2/time.hpp>


namespace program0 { 


u8               getContextID();
util2::Time::Timestamp& getFrameTime();
void initializeLibrary();
void destroyLibrary();
void initializeGraphics();
void destroyGraphics();
void render();


}