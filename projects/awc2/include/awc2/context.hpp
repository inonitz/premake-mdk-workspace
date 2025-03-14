#ifndef __AWC2_CONTEXT_INTERFACE_HEADER__
#define __AWC2_CONTEXT_INTERFACE_HEADER__
#include "C/awc2_api.h"
#include "util/macro.h"
#include "window_types.hpp"


namespace AWC2 {
    static constexpr unsigned int k_maximumContexts = 32; /* I use a u32 bitmap to track the state of each context. meaning, no more than 16 concurrent contexts. */


    struct ContextBitmap
    {
        /*
            [NOTE]: k_maximumContexts is in entry.hpp -
                we can't pull it into here because each awc2*.hpp header is meant to include different parts of the library ('island header')

            activeId - the id (given to user, actually real_id+1) currently active
            libInit  - if the awc2 library has initialized or not. 
                        unfortunately, I have to put this here to pack AWC2Data to 64 Bytes.
            initializedBits - Bitmap of the initialized contexts (1 = init, 0 = not init)
                                By definition, every context that has gone through initialization will be used

            upon calling closeContext(...), the context will be erased from the initializedBits bitmap
        */
        u32 initializedBits{0};
        u8  activeId{0xFF};
        u8  libInit{DEFAULT8};
        u8  reserved[2]{DEFAULT8};
    };


    struct ContextList {
        u8 id[k_maximumContexts];
        u8 actualSize;
        u8 reserved[7];


        u8* begin() { return &id[0];          }
        u8* end()   { return &id[actualSize]; }
        bool empty() const { return actualSize == 0; }
    };


    struct ViewportSize {
        u32 x;
        u32 y;
    };
    struct alignsz(16) ContextDescriptor
    {
        using __WinDesc = WindowDescriptor;
        u8        id;
        u8        reserved[1];
        u16       winWidth;
        u16       winHeight;
        __WinDesc winDesc;
    };

    
    using ContextID  = unsigned char;
    using ContextMap = u32;


    UTIL_STATIC_ASSERT(sizeof(ContextID)         == 1,  ContextID,         "Conforming Size Types Across Compilers")
    UTIL_STATIC_ASSERT(sizeof(ContextMap)        == 4,  ContextMap,        "Conforming Size Types Across Compilers")
    UTIL_STATIC_ASSERT(sizeof(ContextList)       == 40, ContextList,       "Conforming Size Types Across Compilers")
    UTIL_STATIC_ASSERT(sizeof(ViewportSize)      == 8,  ViewportSize,      "Conforming Size Types Across Compilers")
    UTIL_STATIC_ASSERT(sizeof(ContextDescriptor) == 16, ContextDescriptor, "Conforming Size Types Across Compilers")



    AWC2_API ContextID createContext();
    AWC2_API void      destroyContext(ContextID id);
    AWC2_API void      initializeContext(
        ContextID id, 
        u16 windowWidth,
        u16 windowHeight,
        WindowDescriptor const& win_desc
    );
    AWC2_API void      initializeContext(
        const ContextDescriptor& descriptor
    );
    AWC2_API void      closeContext(ContextID id);
    
    AWC2_API ContextMap         getActiveContextBitmap();
    AWC2_API ContextList        getActiveContextList();
    AWC2_API __hot bool         getContextStatus(ContextID id); /* If context is closed or not */
    AWC2_API __hot void         setCurrentContext(ContextID id = 0);
    AWC2_API __hot ContextID    getCurrentContext();
    AWC2_API __hot ViewportSize getCurrentContextViewport();
    AWC2_API WindowStateFlag    getCurrentContextWindowState();
    AWC2_API __hot void begin();
    AWC2_API __hot void end();
} // namespace AWC2


#endif