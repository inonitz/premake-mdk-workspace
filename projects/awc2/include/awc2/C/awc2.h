#ifndef __AWC2_C_COLLECTIVE_INTERFACE_HEADER__
#define __AWC2_C_COLLECTIVE_INTERFACE_HEADER__
#include "awc2_api.h"
#include "input.h"
#include "context.h"
#include "event.h"
#if defined __cplusplus
#   define __AWC2_C_INTERFACE_BEGIN__ extern "C" {
#   define __AWC2_C_INTERFACE_END__ }
#else
#   define __AWC2_C_INTERFACE_BEGIN__
#   define __AWC2_C_INTERFACE_END__
#endif


__AWC2_C_INTERFACE_BEGIN__


/* entry.hpp */
/* namespace AWC2 { */
AWC2_API void awc2init();
AWC2_API void awc2destroy();
AWC2_API void awc2newframe();
/* } */


/* input.hpp */
/* namespace AWC2::Input { */
AWC2_API bool_t awc2isKeyPressed(AWC2KeyCode key);
AWC2_API bool_t awc2isKeyReleased(AWC2KeyCode key);
AWC2_API bool_t awc2isKeyRepeated(AWC2KeyCode key);
AWC2_API bool_t awc2isMouseButtonPressed (AWC2MouseButton mb);
AWC2_API bool_t awc2isMouseButtonReleased(AWC2MouseButton mb);
AWC2_API bool_t awc2isMouseMoving();
AWC2_API bool_t awc2isMouseScrollMoving();
AWC2_API AWC2CursorPosition awc2getMouseScrollOffset();
AWC2_API AWC2CursorPosition awc2getMousePosition();
AWC2_API AWC2CursorPosition awc2getMousePositionDelta();
AWC2_API void awc2setCursorMode(AWC2CursorMode mode);
/* } */


/* context.hpp */
/* namespace AWC2 { */
AWC2_API AWC2ContextID           awc2createContext();
AWC2_API void                    awc2destroyContext(AWC2ContextID id);
AWC2_API void                    awc2initializeContext(AWC2ContextDescriptor const* ctxt);
AWC2_API void                    awc2closeContext(AWC2ContextID id);
AWC2_API void                    awc2getActiveContextList(AWC2ContextVector* outbuf);
AWC2_API __hot bool_t              awc2getContextStatus(AWC2ContextID id); /* If context is closed or not */
AWC2_API __hot void              awc2setCurrentContext(AWC2ContextID id);
AWC2_API __hot AWC2ContextID     awc2getCurrentContext();
AWC2_API __hot AWC2ViewportSize  awc2getCurrentContextViewport();
AWC2_API    AWC2WindowStateFlag  awc2getCurrentContextWindowState();
AWC2_API __hot void              awc2begin();
AWC2_API __hot void              awc2end();
/* } */


/* event.hpp */
/* namespace AWC2 { */
AWC2_API void awc2setUserCallbackWindowSize   (AWC2User_callback_window_size    );
AWC2_API void awc2setUserCallbackKeyboard	  (AWC2User_callback_keyboard 	    );
AWC2_API void awc2setUserCallbackWindowFocus  (AWC2User_callback_window_focus   );
AWC2_API void awc2setUserCallbackMousePosition(AWC2User_callback_mouse_pos 	    );
AWC2_API void awc2setUserCallbackMouseButton  (AWC2User_callback_mouse_button   );
AWC2_API void awc2setUserCallbackMouseScroll  (AWC2User_callback_mouse_scroll   );
AWC2_API void awc2setContextUserCallbackWindowSize   (u8, AWC2User_callback_window_size  );
AWC2_API void awc2setContextUserCallbackKeyboard	 (u8, AWC2User_callback_keyboard 	 );
AWC2_API void awc2setContextUserCallbackWindowFocus  (u8, AWC2User_callback_window_focus );
AWC2_API void awc2setContextUserCallbackMousePosition(u8, AWC2User_callback_mouse_pos 	 );
AWC2_API void awc2setContextUserCallbackMouseButton	 (u8, AWC2User_callback_mouse_button );
AWC2_API void awc2setContextUserCallbackMouseScroll	 (u8, AWC2User_callback_mouse_scroll );
/* } */


__AWC2_C_INTERFACE_END__


#endif /* __AWC2_C_COLLECTIVE_INTERFACE_HEADER__ */