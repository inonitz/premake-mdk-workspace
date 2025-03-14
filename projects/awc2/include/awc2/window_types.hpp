#ifndef __AWC2_WINDOW_TYPES_HEADER__
#define __AWC2_WINDOW_TYPES_HEADER__
#include <util/macro.h>
#include <util/base_type.h>
#include <util/static_assert.h>


namespace AWC2 {


enum class WindowCreationFlag : u8 {
    STARTUP_VISIBLE       = 0b0000'0001,
    STARTUP_FOCUSED       = 0b0000'0010,
    STARTUP_CENTER_CURSOR = 0b0000'0100,
    RESIZABLE             = 0b0000'1000,
    BORDER                = 0b0001'0000,
    BORDERLESS            = 0b0000'0000,
    RAW_MOUSE_MOTION      = 0b0010'0000,
    USE_VSYNC             = 0b0100'0000,
    DEFAULT               = 0b0011'1101,
    MAX                   = 0b0100'0000
};
inline WindowCreationFlag operator&(WindowCreationFlag flagA, WindowCreationFlag flagB) {
    return __scast(WindowCreationFlag, __scast(u8, flagA) & __scast(u8, flagB) );
}
inline WindowCreationFlag operator|(WindowCreationFlag flagA, WindowCreationFlag flagB) {
    return __scast(WindowCreationFlag, __scast(u8, flagA) | __scast(u8, flagB) );
}
inline WindowCreationFlag operator&=(WindowCreationFlag& flagA, WindowCreationFlag flagB) {
    flagA = flagA & flagB;
    return flagA;
}
inline WindowCreationFlag operator|=(WindowCreationFlag& flagA, WindowCreationFlag flagB) {
    flagA = flagA | flagB;
    return flagA;
}


enum class WindowStateFlag : u8 {
    DEFAULT      = 0b0000'0000,
    MINIMIZED    = 0b0000'0001,
    SIZE_CHANGED = 0b0000'0010,
    FOCUSED      = 0b0000'0100,
    MAX          = 0b0000'1000
};
static inline WindowStateFlag from_conditional(WindowStateFlag flagA, bool condition) {
    return __scast(WindowStateFlag, __scast(u8, flagA) * condition );
}
inline WindowStateFlag operator&(WindowStateFlag flagA, WindowStateFlag flagB) {
    return __scast(WindowStateFlag, __scast(u8, flagA) & __scast(u8, flagB) );
}
inline WindowStateFlag operator|(WindowStateFlag flagA, WindowStateFlag flagB) {
    return __scast(WindowStateFlag, __scast(u8, flagA) | __scast(u8, flagB) );
}
inline WindowStateFlag operator~(WindowStateFlag flagA) {
    return __scast(WindowStateFlag, ~__scast(u8, flagA) );
}
inline WindowStateFlag operator&=(WindowStateFlag& flagA, WindowStateFlag flagB) {
    flagA = flagA & flagB;
    return flagA;
}
inline WindowStateFlag operator|=(WindowStateFlag& flagA, WindowStateFlag flagB) {
    flagA = flagA | flagB;
    return flagA;
}

constexpr u32 bitsPerFramebufferChannel(
    u8 Depth   = 24u, 
    u8 Stencil = 8u, 
    u8 Red     = 8u, 
    u8 Green   = 8u, 
    u8 Blue    = 8u, 
    u8 Alpha   = 8u
) {
    return (Depth << 25) | (Stencil << 20) 
    | (Red   << 15) 
    | (Green << 10) 
    | (Blue  << 5) 
    | (Alpha << 0);
}


struct alignsz(8) WindowDescriptor {
    u32                framebufferChannels;
    u16                refreshRate;
    WindowCreationFlag createFlags;
    WindowStateFlag    stateFlags;


    WindowDescriptor() : 
        framebufferChannels(bitsPerFramebufferChannel()),
        refreshRate(60),
        createFlags(WindowCreationFlag::DEFAULT),
        stateFlags(WindowStateFlag::DEFAULT) {}
    
    WindowDescriptor(
        u32 framebuffer_bits_per_channel,
        u16 refresh_rate,
        WindowCreationFlag const& setup_flags
    ) :
        framebufferChannels(framebuffer_bits_per_channel),
        refreshRate(refresh_rate),
        createFlags(setup_flags),
        stateFlags() {}

    WindowDescriptor& operator=(u64 wd_bits) {
        framebufferChannels = wd_bits & 0xFFFF'FFFF;
        refreshRate = (wd_bits >> 32) & 0xFFFF;
        createFlags = __scast(WindowCreationFlag, (wd_bits >> 48) & 0xFF);
        stateFlags  = __scast(WindowStateFlag,    (wd_bits >> 56) & 0xFF);
        return *this;
    }
};
UTIL_STATIC_ASSERT(sizeof(WindowDescriptor) == 8, WindowDescriptor, "Conforming Size Types Across Compilers")


} // namespace AWC2


#endif