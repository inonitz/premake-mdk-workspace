#ifndef __UTIL2_PRINT_HEADER__
#define __UTIL2_PRINT_HEADER__
#include "util2/C/util2_api.h"
#include "util2/C/base_type.h"
#ifndef __UTIL_PRINT_USING_COLORED_CONSOLE__
    #define __UTIL_PRINT_USING_COLORED_CONSOLE__ 0
#endif


namespace UTIL2_DEPRECATED_MSG("Do not Use this header. It is incomplete and not functioning") util2 {


enum OutputStreamType {
    STANDARD_OUTPUT = 0x00,
    STANDARD_ERROR  = 0x01,
    CUSTOM          = 0x02,
    OUTPUT_STREAM_TYPE_MAX = 0x03
};
void UTIL2_API selectOutputStream(OutputStreamType stype, void* custom_stream = nullptr);
void UTIL2_API print(const char* str);
void UTIL2_API printfmt(const char* fmt, ...);


#if __UTIL_PRINT_USING_COLORED_CONSOLE__ == 1
namespace Terminal {


union Color
{
    DISABLE_WARNING_PUSH
    DISABLE_WARNING_GNU_ANON_STRUCT
    struct {
        u8 red;
        u8 green;
        u8 blue;
    };
    u8 rgb[3];
    DISABLE_WARNING_POP


    Color() : red{0}, green{0}, blue{0} {}
    Color(u8 r, u8 g, u8 b) : red{r}, green{g}, blue{b} {}


    Color operator|(Color const& other) const {
        return { 
            __scast(u8, red   | other.red  ), 
            __scast(u8, green | other.green), 
            __scast(u8, blue  | other.blue ) 
        };
    }

    Color& operator|=(Color const& other) {
        red   |= other.red  ; 
        green |= other.green; 
        blue  |= other.blue ;
        return *this;
    }


    static Color black() { return { 0x00, 0x00, 0x00 }; }
    static Color white() { return { 0xff, 0xff, 0xff }; }
};


enum TextAttributes : unsigned char {
    NORMAL = 0,
    ITALIC = 1,
    BOLD   = 2,
    TEXT_ATTRIBUTE_MAX = 3
};


void setColor(
    bool           text_or_background,
    TextAttributes textType  = TextAttributes::NORMAL,
    Color const&   color_rgb = Color::white()
);

void setToDefault(bool text_or_background);


} /* namespace Terminal */


#endif /* __UTIL_PRINT_USING_COLORED_CONSOLE__ */


} /* namespace util2 */


#endif /* __UTIL2_PRINT_HEADER__ */