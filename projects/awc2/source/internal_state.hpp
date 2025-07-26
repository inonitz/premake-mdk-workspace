#ifndef __AWC2_INTERNAL_STATE_DEFINITION_HEADER__
#define __AWC2_INTERNAL_STATE_DEFINITION_HEADER__
#include "awc2/context.hpp"
#include "internal_event.hpp"
#include "internal_window.hpp"
#include "internal_input.hpp"
#include <util2/pool.hpp>
#include <util2/bufferptr.hpp>


namespace AWC2::internal {


void unbindActiveContext();


struct alignsz(32) AWC2ContextData
{
    userCallbackTable m_event_table;
    Window            m_window;
    InputState        m_io;
    void*             m_imgui{DEFAULTVOIDPTR};
    u8                m_id{DEFAULT8};
    u8                m_FlagInit{DEFAULT8};
    u8                m_FlagClosed{DEFAULT8};
    u8                reserved[6]{0};

    void create(
        u8 __internal_context_id,
        u16 __win_width, 
        u16 __win_height, 
        WindowDescriptor const& __win_desc
    );
    void initialize();
    void close(); 
    void destroy();
    void setCurrent() const;
    void begin();
    void end();
};

/* 
    Need to update Struct definiton based on usage patterns in the code 
    Whatever is used more frequently should be put in the first cache line (64 bytes accessed)
*/
// struct alignsz(32) AWC2ContextData_Ver2 {
//     InputState  m_io;
//     GLFWwindow* m_glfw;
//     void*       m_imgui{DEFAULTVOIDPTR};
// };


// struct AWC2Data
// {
//     util2::BufferPointer<AWC2ContextData> poolmem;
//     u8    activeid = DEFAULT8;
//     bool  init     = false;
//     u8    reserved[6]{0};
//     Pool<sizeof(AWC2ContextData), true> ctxpool;
//     // AWC2::ContextBitmap contextStatus;
//     // AWC2::ContextVector initializedContexts;
//     // AWC2::ContextVector inactiveContexts;
// };


struct AWC2Data {
    using nonOwningDataPool = util2::Pool<sizeof(AWC2ContextData), true>;
    using owningBufferPtr   = util2::BufferPointer<AWC2ContextData>;
 
    nonOwningDataPool ctxpool;
    owningBufferPtr   poolmem;
    AWC2::ContextBitmap ctxmap;
};


} // namespace AWC2::internal


#endif