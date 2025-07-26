#include "awc2/C/awc2.h"
#include "awc2/context.hpp"
#include "awc2/entry.hpp"
#include "awc2/input.hpp"
#include "../internal_event.hpp"
#include "../internal_instance.hpp"
#include "../internal_state.hpp"
#include <util2/C/static_assert.h>
#ifdef __cplusplus
#   define AWC2_EXTERNC extern "C"
#else
#   define AWC2_EXTERNC
#endif


UTIL2_STATIC_ASSERT(
    sizeof(AWC2KeyCode) == sizeof(AWC2::Input::keyCode), 
    InputKeyCodeType_SizeEqualityCheck, ""
);
UTIL2_STATIC_ASSERT(
    sizeof(AWC2MouseButton) == sizeof(AWC2::Input::mouseButton),
    InputMouseButtonType_SizeEqualityCheck, ""
);
UTIL2_STATIC_ASSERT(
    sizeof(AWC2CursorPosition) == sizeof(AWC2::Input::cursorPosition),
    InputMouseButtonType_SizeEqualityCheck, ""
);
UTIL2_STATIC_ASSERT(
    sizeof(AWC2CursorPosition64) == sizeof(AWC2::Input::cursorPosition64),
    InputCursorPositionType_SizeEqualityCheck, ""
);
UTIL2_STATIC_ASSERT(
    sizeof(AWC2CursorMode) == sizeof(AWC2::Input::cursorMode),
    InputCursorModeType_SizeEqualityCheck, ""
);
UTIL2_STATIC_ASSERT(
    sizeof(AWC2ContextID) == sizeof(AWC2::ContextID),
    ContextIDType_SizeEqualityCheck, ""
);
UTIL2_STATIC_ASSERT(
    sizeof(AWC2ContextDescriptor) == sizeof(AWC2::ContextDescriptor),
    ContextDescriptionType_SizeEqualityCheck, ""
);


/* namespace AWC2 { */
AWC2_EXTERNC void awc2init() {
    AWC2::init();
}
AWC2_EXTERNC void awc2destroy() {
    AWC2::destroy();
}
AWC2_EXTERNC void awc2newframe() {
    AWC2::newFrame();
}
/* } */


/* namespace AWC2::Input { */
AWC2_EXTERNC bool_t awc2isKeyPressed(AWC2KeyCode key) {
    return AWC2::Input::isKeyPressed(__scast(AWC2::Input::keyCode, key));
}

AWC2_EXTERNC bool_t awc2isKeyReleased(AWC2KeyCode key) {
    return AWC2::Input::isKeyReleased(__scast(AWC2::Input::keyCode, key));
}

AWC2_EXTERNC bool_t awc2isKeyRepeated(AWC2KeyCode key) {
    return AWC2::Input::isKeyRepeated(__scast(AWC2::Input::keyCode, key));
}

AWC2_EXTERNC bool_t awc2isMouseButtonPressed (AWC2MouseButton mb) {
    return AWC2::Input::isMouseButtonPressed(__scast(AWC2::Input::mouseButton, mb));
}

AWC2_EXTERNC bool_t awc2isMouseButtonReleased(AWC2MouseButton mb) {
    return AWC2::Input::isMouseButtonReleased(__scast(AWC2::Input::mouseButton, mb));
}

AWC2_EXTERNC bool_t awc2isMouseMoving() {
    return AWC2::Input::isMouseMoving();
}

AWC2_EXTERNC bool_t awc2isMouseScrollMoving() {
    return AWC2::Input::isMouseScrollMoving();
}

AWC2_EXTERNC AWC2CursorPosition awc2getMouseScrollOffset() {
    /* They're essentially the same type so this should be optimized out */
    AWC2::Input::cursorPosition scrollOffCpp = AWC2::Input::getMouseScrollOffset();
    AWC2CursorPosition          scrollOff = { {{scrollOffCpp.x, scrollOffCpp.y}} };
    return scrollOff;
}

AWC2_EXTERNC AWC2CursorPosition awc2getMousePosition() {
    /* They're essentially the same type so this should be optimized out */
    AWC2::Input::cursorPosition cursorPosCpp = AWC2::Input::getMousePosition();
    AWC2CursorPosition          cursorPos    = { {{cursorPosCpp.x, cursorPosCpp.y}} };
    return cursorPos;
}

AWC2_EXTERNC AWC2CursorPosition awc2getMousePositionDelta() {
    /* They're essentially the same type so this should be optimized out */
    AWC2::Input::cursorPosition curPosDiffCpp = AWC2::Input::getMousePositionDelta();
    AWC2CursorPosition          curPosDiff    = { {{curPosDiffCpp.x, curPosDiffCpp.y}} };
    return curPosDiff;
}

AWC2_EXTERNC void awc2setCursorMode(AWC2CursorMode mode) {
    AWC2::Input::setCursorMode(__scast(AWC2::Input::cursorMode, mode));
    return;
}
/* } */


/* namespace AWC2 { */
AWC2_EXTERNC AWC2ContextID awc2createContext() 
{
    return AWC2::createContext();
}

AWC2_EXTERNC void awc2destroyContext(AWC2ContextID id) 
{
    return AWC2::destroyContext(id);
}

AWC2_EXTERNC void awc2initializeContext(AWC2ContextDescriptor const* ctxt) 
{
    AWC2::ContextDescriptor cppdesc = {
        ctxt->id,
        { ctxt->reserved[0] },
        ctxt->winWidth,
        ctxt->winHeight,
        AWC2::WindowDescriptor{
            ctxt->winDesc.framebufferChannels,
            ctxt->winDesc.refreshRate,
            AWC2::WindowCreationFlag{ctxt->winDesc.createFlags}
        }
    };
    AWC2::initializeContext(cppdesc);
    return;
}

AWC2_EXTERNC void awc2closeContext(AWC2ContextID id) 
{
    AWC2::closeContext(id);
}

AWC2_EXTERNC void awc2getActiveContextList(AWC2ContextVector* outbuf) 
{
    auto initbitmap = AWC2::internal::__awc2_lib_get_instance()->ctxmap.initializedBits;
    u8 cond = 0;
    // outbuf->actualSize = 0;
    // for(u8 i = 0; i < 8 * sizeof(initbitmap); ++i) {
    //     cond = (initbitmap >> i) & 0b1;
    //     outbuf->id[outbuf->actualSize] = i + 1;
    //     outbuf->actualSize += cond;
    // }
    // outbuf->id[ outbuf->actualSize - (outbuf->actualSize == AWC2_K_MAXIMUM_CONTEXTS) ] = cond ? 32 : 0;
    
    outbuf->actualSize = 0;
    for(u8 i = 0; i < 8 * sizeof(initbitmap); ++i) {
        cond = (initbitmap >> i) & 0b1;
        if(cond) {
            outbuf->id[outbuf->actualSize] = i + 1;
            ++outbuf->actualSize;
        }
    }
    return;
}

AWC2_EXTERNC __hot bool_t awc2getContextStatus(AWC2ContextID id) 
{
    return AWC2::getContextStatus(id);
}

AWC2_EXTERNC __hot void awc2setCurrentContext(AWC2ContextID id) 
{
    AWC2::setCurrentContext(id);
    return;
}

AWC2_EXTERNC __hot AWC2ContextID awc2getCurrentContext()
{
    return AWC2::getCurrentContext();
}

AWC2_EXTERNC __hot AWC2ViewportSize awc2getCurrentContextViewport() 
{
    auto cpp_viewportsize = AWC2::getCurrentContextViewport();
    return AWC2ViewportSize{ cpp_viewportsize.x, cpp_viewportsize.y };

}

AWC2_EXTERNC u8 awc2getCurrentContextWindowState()
{
    return AWC2WindowStateFlag{
        __scast(u8, AWC2::getCurrentContextWindowState())
    };
}


AWC2_EXTERNC __hot void awc2begin() {
    AWC2::begin();
    return;
}

AWC2_EXTERNC __hot void  awc2end() {
    AWC2::end();
    return;
}
/* } */


#define __common_func_active_ctxt(func_index, handler_address) \
    u64 handlerAddressu64 = (handlerAddress == nullptr) \
        ? __rcast(up64, AWC2::internal::user_callback_func_noop) \
            : \
            __rcast(up64, handlerAddress); \
        \
    AWC2::internal::__awc2_lib_get_context( \
        AWC2::internal::__awc2_lib_get_active_context_id() \
    ).m_event_table \
        .pointers[func_index] = handlerAddressu64;


#define __common_func(context_id, func_index, handler_address) \
    u64 handlerAddressu64 = (handlerAddress == nullptr) \
        ? __rcast(up64, AWC2::internal::user_callback_func_noop) \
            : \
            __rcast(up64, handlerAddress); \
        \
    AWC2::internal::__awc2_lib_get_context(context_id).m_event_table \
        .pointers[func_index] = handlerAddressu64; \


/* namespace AWC2 { */
void awc2setUserCallbackWindowSize   (AWC2User_callback_window_size    handlerAddress) {
    __common_func_active_ctxt(1 - 1, __rcast(up64, handlerAddress))
}
void awc2setUserCallbackKeyboard	 (AWC2User_callback_keyboard 	   handlerAddress) {
    __common_func_active_ctxt(2 - 1, __rcast(up64, handlerAddress))
}
void awc2setUserCallbackWindowFocus  (AWC2User_callback_window_focus   handlerAddress) {
    __common_func_active_ctxt(3 - 1, __rcast(up64, handlerAddress))
}
void awc2setUserCallbackMousePosition(AWC2User_callback_mouse_pos 	   handlerAddress) {
    __common_func_active_ctxt(4 - 1, __rcast(up64, handlerAddress))
}
void awc2setUserCallbackMouseButton	 (AWC2User_callback_mouse_button   handlerAddress) {
    __common_func_active_ctxt(5 - 1, __rcast(up64, handlerAddress))
}
void awc2setUserCallbackMouseScroll	 (AWC2User_callback_mouse_scroll   handlerAddress) {
    __common_func_active_ctxt(6 - 1, __rcast(up64, handlerAddress))
}
void awc2setContextUserCallbackWindowSize   (u8 context_id, AWC2User_callback_window_size handlerAddress) {
    __common_func(context_id, 1 - 1, __rcast(up64, handlerAddress))
}
void awc2setContextUserCallbackKeyboard	    (u8 context_id, AWC2User_callback_keyboard  	handlerAddress) {
    __common_func(context_id, 2 - 1, __rcast(up64, handlerAddress))
}
void awc2setContextUserCallbackWindowFocus  (u8 context_id, AWC2User_callback_window_focus  handlerAddress) {
    __common_func(context_id, 3 - 1, __rcast(up64, handlerAddress))
}
void awc2setContextUserCallbackMousePosition(u8 context_id, AWC2User_callback_mouse_pos  	handlerAddress) {
    __common_func(context_id, 4 - 1, __rcast(up64, handlerAddress))
}
void awc2setContextUserCallbackMouseButton	(u8 context_id, AWC2User_callback_mouse_button  handlerAddress) {
    __common_func(context_id, 5 - 1, __rcast(up64, handlerAddress))
}
void awc2setContextUserCallbackMouseScroll	(u8 context_id, AWC2User_callback_mouse_scroll  handlerAddress) {
    __common_func(context_id, 6 - 1, __rcast(up64, handlerAddress))
}
/* } */