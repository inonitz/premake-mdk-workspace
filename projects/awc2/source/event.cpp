#include "awc2/event.hpp"
#include "internal_event.hpp"
#include "internal_instance.hpp"
#include "internal_state.hpp"
#include <util2/C/marker4.h>


namespace AWC2 {


void __common_set_user_func(u8 context_id, u8 func_index, u64 handler_address_bits)
{
    internal::__awc2_lib_get_context(context_id).m_event_table
        .pointers[func_index] = handler_address_bits;
    return;
}


template<class FuncPtr> void setUserCallback(FuncPtr handlerAddress) 
{
    setContextUserCallback(internal::__awc2_lib_get_active_context_id(), handlerAddress);
    return;
}


template<class FuncPtr> void setContextUserCallback(u8 context_id, FuncPtr handlerAddress)
{
    u64 func_addr = (handlerAddress == nullptr) ? 
        __rcast(up64, internal::user_callback_func_noop) : 
        __rcast(up64, handlerAddress);
    __common_set_user_func(
        context_id,
        internal::UserFuncIndexer<FuncPtr>()(), 
        func_addr
    );
    return;
}



template void setUserCallback<user_callback_window_size> (user_callback_window_size );
template void setUserCallback<user_callback_keyboard>	 (user_callback_keyboard 	);
template void setUserCallback<user_callback_window_focus>(user_callback_window_focus);
template void setUserCallback<user_callback_mouse_pos>	 (user_callback_mouse_pos 	);
template void setUserCallback<user_callback_mouse_button>(user_callback_mouse_button);
template void setUserCallback<user_callback_mouse_scroll>(user_callback_mouse_scroll);
template void setContextUserCallback<user_callback_window_size>  (u8, user_callback_window_size );
template void setContextUserCallback<user_callback_keyboard>	 (u8, user_callback_keyboard 	);
template void setContextUserCallback<user_callback_window_focus> (u8, user_callback_window_focus);
template void setContextUserCallback<user_callback_mouse_pos>	 (u8, user_callback_mouse_pos 	);
template void setContextUserCallback<user_callback_mouse_button> (u8, user_callback_mouse_button);
template void setContextUserCallback<user_callback_mouse_scroll> (u8, user_callback_mouse_scroll);


} // namespace AWC2