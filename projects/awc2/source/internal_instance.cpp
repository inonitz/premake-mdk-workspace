#include "internal_instance.hpp"
#include "internal_state.hpp"


namespace AWC2::internal {


static AWC2Data __library_local_data;


AWC2Data* __awc2_lib_get_instance()
{
    return &__library_local_data;
}

AWC2ContextData& __awc2_lib_get_context(unsigned char id) {
    ifcrashfmt_debug(id == 0xFF, "No Active Context Selected!\n");
    return __library_local_data.poolmem[--id];
}

AWC2ContextData& __awc2_lib_get_active_context()
{
    return __awc2_lib_get_context(__awc2_lib_get_active_context_id());
}

unsigned char __awc2_lib_get_active_context_id()
{
    return __library_local_data.ctxmap.activeId; /* 0xFF = no context active; will be easily seen in debugger */
}


} // namespace AWC2::internal