#ifndef __AWC2_C_CONTEXT_INTERFACE_HEADER__
#define __AWC2_C_CONTEXT_INTERFACE_HEADER__
#include <util/macro.h>
#include <util/base_type.h>


#ifdef __cplusplus
extern "C" {
#endif


#define AWC2_K_MAXIMUM_CONTEXTS 32


typedef u8 AWC2WindowCreationFlag;
#define AWC2_WINDOW_CREATION_FLAG_STARTUP_VISIBLE       (0b00000001)
#define AWC2_WINDOW_CREATION_FLAG_STARTUP_FOCUSED       (0b00000010)
#define AWC2_WINDOW_CREATION_FLAG_STARTUP_CENTER_CURSOR (0b00000100)
#define AWC2_WINDOW_CREATION_FLAG_RESIZABLE             (0b00001000)
#define AWC2_WINDOW_CREATION_FLAG_BORDER                (0b00010000)
#define AWC2_WINDOW_CREATION_FLAG_BORDERLESS            (0b00000000)
#define AWC2_WINDOW_CREATION_FLAG_RAW_MOUSE_MOTION      (0b00100000)
#define AWC2_WINDOW_CREATION_FLAG_USE_VSYNC             (0b01000000)
#define AWC2_WINDOW_CREATION_FLAG_DEFAULT               (0b00111101)
#define AWC2_WINDOW_CREATION_FLAG_MAX                   (0b01000000)


typedef u8 AWC2WindowStateFlag;
#define AWC2_WINDOW_STATE_FLAG_DEFAULT      (0b00000000)
#define AWC2_WINDOW_STATE_FLAG_MINIMIZED    (0b00000001)
#define AWC2_WINDOW_STATE_FLAG_SIZE_CHANGED (0b00000010)
#define AWC2_WINDOW_STATE_FLAG_FOCUSED      (0b00000100)
#define AWC2_WINDOW_STATE_FLAG_MAX          (0b00001000)


#if !defined AWC2BitsPerFramebufferChannel
#   define AWC2BitsPerFramebufferChannel(Depth, Stencil, Red, Green, Blue, Alpha) \
        (Depth << 25) | (Stencil << 20) \
            | (Red   << 15) \
            | (Green << 10) \
            | (Blue  <<  5) \
            | (Alpha <<  0)
#else
#   warning "AWC2BitsPerFramebufferChannel Shouldn't be already defined. Something went wrong"
#endif


typedef struct alignsz(8) __awc2_WindowDescriptorStructure
{
    u32                    framebufferChannels;
    u16                    refreshRate;
    AWC2WindowCreationFlag createFlags;
    AWC2WindowStateFlag    stateFlags;
} AWC2WindowDescriptor;
UTIL_STATIC_ASSERT(sizeof(AWC2WindowDescriptor) == 8, WindowDescriptor, "Conforming Size Types Across Compilers")


__force_inline inline u64 awc2WindowDescriptorAsU64(AWC2WindowDescriptor* wdesc) {
    return *((u64*)wdesc);
}
__force_inline inline void awc2WindowDescriptorDefault(AWC2WindowDescriptor* wdesc) {
    wdesc->framebufferChannels = AWC2BitsPerFramebufferChannel(24u, 8u, 8u, 8u, 8u, 8u);
    wdesc->refreshRate = 60;
    wdesc->createFlags = AWC2_WINDOW_CREATION_FLAG_DEFAULT;
    wdesc->stateFlags  = AWC2_WINDOW_STATE_FLAG_DEFAULT;
    return;
}


typedef u8 AWC2ContextID;
typedef struct alignsz(8) __awc2_WindowViewport_DimensionsStructure
{
    u32 x;
    u32 y;
} AWC2ViewportSize;


typedef struct alignsz(8) __awc2_ContextIDBufferStructure
{
    AWC2ContextID id[AWC2_K_MAXIMUM_CONTEXTS];
    u8            actualSize;
    u8            reserved[7];
} AWC2ContextVector;


typedef struct alignsz(16) __awc2_ContextDescriptorStructure
{
    u8        id;
    u8        reserved[1];
    u16       winWidth;
    u16       winHeight;
    AWC2WindowDescriptor winDesc;
} AWC2ContextDescriptor;


UTIL_STATIC_ASSERT(sizeof(AWC2ContextID)          == 1,  AWC2ContextID,          "Conforming Size Types Across Compilers")
UTIL_STATIC_ASSERT(sizeof(AWC2ContextVector)      == 40, AWC2ContextVector,      "Conforming Size Types Across Compilers")
UTIL_STATIC_ASSERT(sizeof(AWC2ViewportSize)       == 8,  AWC2ViewportSize,       "Conforming Size Types Across Compilers")
UTIL_STATIC_ASSERT(sizeof(AWC2ContextDescriptor)  == 16, AWC2ContextDescriptor,  "Conforming Size Types Across Compilers")


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* __AWC2_C_CONTEXT_INTERFACE_HEADER__ */