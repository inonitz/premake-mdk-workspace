#include "awc2/context.hpp"
#include "internal_instance.hpp"
#include "internal_state.hpp"
#include <util2/ifcrash.hpp>


namespace AWC2 {


ContextID createContext() {
    auto& instance = *internal::__awc2_lib_get_instance();
    ifcrashstr_debug(instance.ctxmap.libInit == false, 
        "createContext() => AWC2 Library not initialized! call AWC2::init()\n"
    );
    return instance.ctxpool.allocate_index() + 1;
}


void destroyContext(ContextID id)
{
    ifcrashstr_debug(id == 0 || id > AWC2::k_maximumContexts, 
        "AWC2::destroyContext(ContextID id) => invalid id supplied"
    );
    internal::__awc2_lib_get_context(id).destroy();
    internal::__awc2_lib_get_instance()->ctxpool.free_index(--id);
    return;
}


void initializeContext(
    ContextID id, 
    u16 windowWidth,
    u16 windowHeight,
    WindowDescriptor const& win_desc
) {
    ifcrashstr_debug(id == 0 || id > AWC2::k_maximumContexts, 
        "AWC2::initializeContext(...) => invalid id supplied"
    );
    auto& ctxt = internal::__awc2_lib_get_context(id); 
    ctxt.create(
        id,
        windowWidth, 
        windowHeight, 
        win_desc
    );
    ctxt.initialize();
    /* Context Tracking & Management */
    internal::__awc2_lib_get_instance()->ctxmap.initializedBits |= (1 << (--id));
    return;
}


void initializeContext(
    const ContextDescriptor& descriptor
) {
    initializeContext(
        descriptor.id,
        descriptor.winWidth, 
        descriptor.winHeight, 
        descriptor.winDesc
    );
    return;
}


void closeContext(ContextID id)
{
    /*
        [NOTE]: 
        Closing GLFW windows dynamically doesn't work,
        dynamically meaning in the game-loop, even if the window/its context is unbound;
        [4 days of my life are gone :( ]
        Therefore, the only thing that can be done is to hide it and have a Flag to notify the
        programmer that this windows' context is effectively unusable.
        Also, waiting until AWC2::destroy() to completely release all the windows I previously allocated :/
    */
    auto& instance = *internal::__awc2_lib_get_instance();

    internal::__awc2_lib_get_context(id).close();
    /* Context Tracking & Management */
    instance.ctxmap.initializedBits &= ~(1 << (id-1)); /* bit_position+1 = id; we're resetting the bit associated with the specific id */
    return;
}


ContextMap getActiveContextBitmap()
{
    return internal::__awc2_lib_get_instance()->ctxmap.initializedBits;
}


ContextList getActiveContextList()
{
    auto initbitmap = getActiveContextBitmap();
    u8 cond = 0;
    // outbuf->actualSize = 0;
    // for(u8 i = 0; i < 8 * sizeof(initbitmap); ++i) {
    //     cond = (initbitmap >> i) & 0b1;
    //     outbuf->id[outbuf->actualSize] = i + 1;
    //     outbuf->actualSize += cond;
    // }
    // outbuf->id[ outbuf->actualSize - (outbuf->actualSize == AWC2_K_MAXIMUM_CONTEXTS) ] = cond ? 32 : 0;
    ContextList to_ret;


    to_ret.actualSize = 0;
    for(u8 i = 0; i < 8 * sizeof(initbitmap); ++i) {
        cond = (initbitmap >> i) & 0b1;
        if(cond) {
            to_ret.id[to_ret.actualSize] = i + 1;
            ++to_ret.actualSize;
        }
    }
    return to_ret;
}


__hot bool getContextStatus(ContextID id) {
    return internal::__awc2_lib_get_context(id).m_FlagClosed;
}


__hot void setCurrentContext(ContextID id) 
{
    ifcrashstr_debug(id > AWC2::k_maximumContexts, 
        "AWC2::initializeContext(...) => invalid id supplied"
    );

    if(unlikely(id == 0)) {
        /* unbind everthing */
        /* No way to avoid this as order-of-operations is opposite from binding */
        internal::unbindActiveContext();
    } else {
        internal::__awc2_lib_get_context(id).setCurrent();
    }
    internal::__awc2_lib_get_instance()->ctxmap.activeId = (id == 0) ? 0xFF : id;
    return;
}


__hot ContextID getCurrentContext()
{
    return internal::__awc2_lib_get_active_context_id();
}


__hot ViewportSize getCurrentContextViewport()
{
    return ViewportSize{
        internal::__awc2_lib_get_active_context().m_window.getWidth(),
        internal::__awc2_lib_get_active_context().m_window.getHeight()
    };
}


AWC2_API WindowStateFlag getCurrentContextWindowState()
{
    return internal::__awc2_lib_get_active_context().m_window.getFlags();
}


__hot void begin() { internal::__awc2_lib_get_active_context().begin(); return; }
__hot void end()   { internal::__awc2_lib_get_active_context().end();   return; }


} // namespace AWC2