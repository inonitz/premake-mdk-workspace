#pragma once
#include <util2/C/base_type.h>
#include <util2/time.hpp>


namespace ssbowork {

u8               getContextID();
util2::Time::Timestamp& getFrameTime();
util2::Time::Timestamp& getRenderTime();
util2::Time::Timestamp& getTimer0();
util2::Time::Timestamp& getTimer1();
bool             getSlowRenderFlag();
void initializeLibrary();
void destroyLibrary();
void initializeGraphics();
void destroyGraphics();
void render();

}