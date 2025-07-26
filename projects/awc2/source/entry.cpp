#include "awc2/entry.hpp"
#include "internal_instance.hpp"
#include "internal_state.hpp"
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <util2/C/marker4.h>
#include <util2/C/aligned_malloc.h>
#include <util2/ifcrash.hpp>
#include <util2/string.hpp>
#include <util2/pool.hpp>


namespace AWC2 {


static inline void glfw_err_callback(
    __release_unused int errnum, 
    __release_unused const char* errmsg
) {
    markfmt("GLFW ERROR %u => %s\n", errnum, errmsg);
    return;
}


void init()
{
    auto& __libdata = *internal::__awc2_lib_get_instance();
    
    /* GLFW init */
    glfwSetErrorCallback(glfw_err_callback);
    ifcrashstr(glfwInit() == GLFW_FALSE, "AWC2::init() Failed => glfwInit() returned 0")

    /* ImGui init */
    IMGUI_CHECKVERSION();

    /* AWC2 init */
    /* Allocate data for maximal amount of contexts */
    static constexpr u64 ctxsize = sizeof(internal::AWC2ContextData);
    __libdata.poolmem.create(
        util2_aligned_malloc(AWC2::k_maximumContexts * ctxsize, ctxsize),
        AWC2::k_maximumContexts
    );
    util2::memset(__libdata.poolmem.data(), AWC2::k_maximumContexts);

    /* Init memory-manager (pool-alloc) */
    __libdata.ctxpool.create(__libdata.poolmem.data(), AWC2::k_maximumContexts);
    /* Init book-keeping of contexts */
    __libdata.ctxmap.libInit         = true;
    __libdata.ctxmap.activeId        = 0xFF;
    __libdata.ctxmap.initializedBits = 0;
    return;
}


void destroy()
{
    auto& __libdata = *internal::__awc2_lib_get_instance();
    util2::memset(__rcast(u8*, &__libdata.ctxmap), 
        sizeof(__libdata.ctxmap), 
        __scast(u8, DEFAULT8)
    );
    __libdata.ctxpool.destroy();


    for(auto& context_obj : __libdata.poolmem) {
        if(context_obj.m_FlagInit == 0x1) /* destroy all that exist */
            context_obj.destroy();
    }
    util2_aligned_free(__libdata.poolmem.data());
    __libdata.poolmem.destroy();
    glfwTerminate();
    return;
}


__hot void newFrame()
{
    glfwPollEvents();
    return;
}


} // namespace AWC2