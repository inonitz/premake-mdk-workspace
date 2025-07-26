#ifndef __AWC2_C_EVENT_INTERFACE_HEADER__
#define __AWC2_C_EVENT_INTERFACE_HEADER__
#include "input.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct GLFWwindow GLFWwindow;


typedef struct __awc2_user_callback_winsize_struct 
{
    GLFWwindow* window;
    u8 context_id;
    u8 reserved[3];
    u16 width;
    u16 height;
} AWC2User_callback_winsize_struct;


typedef struct __awc2_user_callback_keyboard_struct
{
    GLFWwindow*    window;
    u8             context_id;
    AWC2KeyCode    keyStroke;
    AWC2InputState action;
    u8          reserved[5];
} AWC2User_callback_keyboard_struct;


typedef struct __awc2_user_callback_winfocus_struct 
{
    GLFWwindow* window;
    u8          context_id;
    bool_t      focused;
    u8 reserved[6];
} AWC2User_callback_winfocus_struct;


typedef struct __awc2_user_callback_mousecursor_struct 
{
    GLFWwindow*             window;
    u8                      context_id;
    u8 reserved0[7];
    AWC2CursorPosition64 pos;
} AWC2User_callback_mousecursor_struct;


typedef struct __awc2_user_callback_mousescroll_struct 
{
    GLFWwindow*             window;
    u8                      context_id;
    u8 reserved[7];
    AWC2CursorPosition64 offset;
} AWC2User_callback_mousescroll_struct;


typedef struct __awc2_user_callback_mousebutton_struct 
{
    GLFWwindow*     window;
    u8              context_id;
    AWC2MouseButton button;
    AWC2InputState  action;
    u8 reserved[5];
} AWC2User_callback_mousebutton_struct;




UTIL2_STATIC_ASSERT(sizeof(AWC2User_callback_winsize_struct)     == 16, ContextDescriptor, "Conforming Size Types Across Compilers");
UTIL2_STATIC_ASSERT(sizeof(AWC2User_callback_keyboard_struct)    == 16, ContextDescriptor, "Conforming Size Types Across Compilers");
UTIL2_STATIC_ASSERT(sizeof(AWC2User_callback_winfocus_struct)    == 16, ContextDescriptor, "Conforming Size Types Across Compilers");
UTIL2_STATIC_ASSERT(sizeof(AWC2User_callback_mousecursor_struct) == 32, ContextDescriptor, "Conforming Size Types Across Compilers");
UTIL2_STATIC_ASSERT(sizeof(AWC2User_callback_mousescroll_struct) == 32, ContextDescriptor, "Conforming Size Types Across Compilers");
UTIL2_STATIC_ASSERT(sizeof(AWC2User_callback_mousebutton_struct) == 16, ContextDescriptor, "Conforming Size Types Across Compilers");
typedef void (*AWC2User_callback_noop		 )(void* generic_pointer);
typedef void (*AWC2User_callback_window_size )(AWC2User_callback_winsize_struct const*);
typedef void (*AWC2User_callback_keyboard	 )(AWC2User_callback_keyboard_struct const*);
typedef void (*AWC2User_callback_window_focus)(AWC2User_callback_winfocus_struct const*);
typedef void (*AWC2User_callback_mouse_pos	 )(AWC2User_callback_mousecursor_struct const*);
typedef void (*AWC2User_callback_mouse_scroll)(AWC2User_callback_mousescroll_struct const*);
typedef void (*AWC2User_callback_mouse_button)(AWC2User_callback_mousebutton_struct const*);


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* __AWC2_C_EVENT_INTERFACE_HEADER__ */