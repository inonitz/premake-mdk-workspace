#ifndef __AWC2_C_INPUT_INTERFACE_HEADER__
#define __AWC2_C_INPUT_INTERFACE_HEADER__
#include <util2/C/macro.h>
#include <util2/C/base_type.h>
#include <util2/C/static_assert.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef u8 AWC2KeyCode;
#define AWC2_KEYCODE_ESCAPE (0)
#define AWC2_KEYCODE_SPACE  (1)
#define AWC2_KEYCODE_NUM0   (2)
#define AWC2_KEYCODE_NUM1   (3)
#define AWC2_KEYCODE_NUM2   (4)
#define AWC2_KEYCODE_NUM3   (5)
#define AWC2_KEYCODE_NUM4   (6)
#define AWC2_KEYCODE_NUM5   (7)
#define AWC2_KEYCODE_NUM6   (8)
#define AWC2_KEYCODE_NUM7   (9)
#define AWC2_KEYCODE_NUM8   (10)
#define AWC2_KEYCODE_NUM9   (11)
#define AWC2_KEYCODE_W 	    (12)
#define AWC2_KEYCODE_A 	    (13)
#define AWC2_KEYCODE_S 	    (14)
#define AWC2_KEYCODE_D 	    (15)
#define AWC2_KEYCODE_Q 	    (16)
#define AWC2_KEYCODE_E 	    (17)
#define AWC2_KEYCODE_R 	    (18)
#define AWC2_KEYCODE_F 	    (19)
#define AWC2_KEYCODE_C 	    (20)
#define AWC2_KEYCODE_X 	    (21)
#define AWC2_KEYCODE_Z 	    (22)
#define AWC2_KEYCODE_T 	    (23)
#define AWC2_KEYCODE_G 	    (24)
#define AWC2_KEYCODE_V 	    (25)
#define AWC2_KEYCODE_B 	    (26)
#define AWC2_KEYCODE_H 	    (27)
#define AWC2_KEYCODE_Y 	    (28)
#define AWC2_KEYCODE_U 	    (29)
#define AWC2_KEYCODE_J 	    (30)
#define AWC2_KEYCODE_N 	    (31)
#define AWC2_KEYCODE_M 	    (32)
#define AWC2_KEYCODE_K 	    (33)
#define AWC2_KEYCODE_I 	    (34)
#define AWC2_KEYCODE_O 	    (35)
#define AWC2_KEYCODE_L 	    (36)
#define AWC2_KEYCODE_P 	    (37)
#define AWC2_KEYCODE_MAX    (38)


typedef u8 AWC2MouseButton;
#define AWC2_MOUSEBUTTON_LEFT   (0)
#define AWC2_MOUSEBUTTON_RIGHT  (1)
#define AWC2_MOUSEBUTTON_MIDDLE (2)
#define AWC2_MOUSEBUTTON_MAX    (3)


typedef u8 AWC2InputState;
#define AWC2_INPUTSTATE_DEFAULT (0) /* mapping is (1 << GLFW_KEY_STATE) */
#define AWC2_INPUTSTATE_RELEASE (1)
#define AWC2_INPUTSTATE_PRESS   (2)
#define AWC2_INPUTSTATE_REPEAT  (4)
#define AWC2_INPUTSTATE_MAX     (8)


typedef u8 AWC2CursorMode;
#define AWC2_CURSORMODE_HIDDEN       (0x00)
#define AWC2_CURSORMODE_NORMAL       (0x01)
#define AWC2_CURSORMODE_DISABLED     (0x02)
#define AWC2_CURSORMODE_SCREEN_BOUND (0x03)
#define AWC2_CURSORMODE_MAX          (0x04)


typedef struct __awc2_genericCursorPosition {
    DISABLE_WARNING_PUSH
    DISABLE_WARNING_GNU_ANON_STRUCT
    DISABLE_WARNING_NESTED_ANON_TYPES
    union 
    {
        struct {
            f32 x, y;
        };
        f32 val[2];
    };
    DISABLE_WARNING_POP
} AWC2CursorPosition;
UTIL2_STATIC_ASSERT(sizeof(AWC2CursorPosition) == 8,  AWC2CursorPositionf32, "Conforming Size Types Across Compilers")


typedef struct __awc2_genericCursorPositionf64 {
    DISABLE_WARNING_PUSH
    DISABLE_WARNING_GNU_ANON_STRUCT
    DISABLE_WARNING_NESTED_ANON_TYPES
    union 
    {
        struct {
            f64 x, y;
        };
        f64 val[2];
    };
    DISABLE_WARNING_POP
} AWC2CursorPosition64;
UTIL2_STATIC_ASSERT(sizeof(AWC2CursorPosition64) == 16, AWC2CursorPositionf64, "Conforming Size Types Across Compilers")


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* __AWC2_C_INPUT_INTERFACE_HEADER__ */