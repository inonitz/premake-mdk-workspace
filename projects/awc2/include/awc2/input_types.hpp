#ifndef __AWC2_INPUT_TYPES_HEADER__
#define __AWC2_INPUT_TYPES_HEADER__
#include <util2/C/macro.h>
#include <util2/type_trait.hpp>



namespace AWC2::Input {


enum class keyCode : u8 {
    ESCAPE,
    SPACE,
    NUM0,
    NUM1,
    NUM2,
    NUM3,
    NUM4,
    NUM5,
    NUM6,
    NUM7,
    NUM8,
    NUM9,
    W,
    A,
    S,
    D,
    Q,
    E,
    R,
    F,
    C,
    X,
    Z,
    T,
    G,
    V,
    B,
    H,
    Y,
    U,
    J,
    N,
    M,
    K,
    I,
    O,
    L,
    P,
    ENUM_MAX
};

enum class mouseButton : u8 { 
    LEFT     = 0, 
    RIGHT    = 1, 
    MIDDLE   = 2, 
    ENUM_MAX = 3 
};

enum class inputState : u8 {
    DEFAULT  = 0, /* mapping is (1 << GLFW_KEY_STATE) */
    RELEASE  = 1,
    PRESS    = 2,
    REPEAT   = 4,
    ENUM_MAX = 8
};


enum class cursorMode : u8 {
    HIDDEN       = 0x00,
    NORMAL       = 0x01,
    DISABLED     = 0x02,
    SCREEN_BOUND = 0x03,
    ENUM_MAX     = 0x04
};


struct cursorPosition {
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
};


struct cursorPosition64 {
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
};


} // namespace AWC2::Input


#endif