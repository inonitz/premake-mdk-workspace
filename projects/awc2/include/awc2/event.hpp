#ifndef __AWC2_EVENT_INTERFACE_HEADER__
#define __AWC2_EVENT_INTERFACE_HEADER__
#include <util/static_assert.h>
#include "C/awc2_api.h"
#include "input_types.hpp"


typedef struct GLFWwindow GLFWwindow;


namespace AWC2
{
    struct user_callback_winsize_struct 
    {
        GLFWwindow* window;
        u8 context_id;
        u8 reserved[3];
        u16 width;
        u16 height;
    };
    struct user_callback_keyboard_struct
    {
        GLFWwindow*       window;
        u8                context_id;
        Input::keyCode    keyStroke;
        Input::inputState action;
        u8          reserved[5];
    };
    struct user_callback_winfocus_struct 
    {
        GLFWwindow* window;
        u8          context_id;
        bool        focused;
        u8 reserved[6];
    };
    struct user_callback_mousecursor_struct 
    {
        GLFWwindow*             window;
        u8                      context_id;
        u8 reserved0[7];
        Input::cursorPosition64 pos;
    };
    struct user_callback_mousescroll_struct 
    {
        GLFWwindow*             window;
        u8                      context_id;
        u8 reserved[7];
        Input::cursorPosition64 offset;
    };
    struct user_callback_mousebutton_struct 
    {
        GLFWwindow*        window;
        u8                 context_id;
        Input::mouseButton button;
        Input::inputState  action;
        u8 reserved[5];
    };


    UTIL_STATIC_ASSERT(sizeof(user_callback_winsize_struct)     == 16, user_callback_winsize_struct,     "Conforming Size Types Across Compilers")
    UTIL_STATIC_ASSERT(sizeof(user_callback_keyboard_struct)    == 16, user_callback_keyboard_struct,    "Conforming Size Types Across Compilers")
    UTIL_STATIC_ASSERT(sizeof(user_callback_winfocus_struct)    == 16, user_callback_winfocus_struct,    "Conforming Size Types Across Compilers")
    UTIL_STATIC_ASSERT(sizeof(user_callback_mousecursor_struct) == 32, user_callback_mousecursor_struct, "Conforming Size Types Across Compilers")
    UTIL_STATIC_ASSERT(sizeof(user_callback_mousescroll_struct) == 32, user_callback_mousescroll_struct, "Conforming Size Types Across Compilers")
    UTIL_STATIC_ASSERT(sizeof(user_callback_mousebutton_struct) == 16, user_callback_mousebutton_struct, "Conforming Size Types Across Compilers")
    typedef void (* user_callback_noop		  )(void* generic_pointer);
    typedef void (* user_callback_window_size )(user_callback_winsize_struct const*);
    typedef void (* user_callback_keyboard	  )(user_callback_keyboard_struct const*);
    typedef void (* user_callback_window_focus)(user_callback_winfocus_struct const*);
    typedef void (* user_callback_mouse_pos	  )(user_callback_mousecursor_struct const*);
    typedef void (* user_callback_mouse_scroll)(user_callback_mousescroll_struct const*);
    typedef void (* user_callback_mouse_button)(user_callback_mousebutton_struct const*);


    template<typename FunctionPointer> AWC2_API void setUserCallback(FunctionPointer = nullptr);
    template<typename FunctionPointer> AWC2_API void setContextUserCallback(u8 context_id, FunctionPointer = nullptr);
}; // namespace AWC2


#endif