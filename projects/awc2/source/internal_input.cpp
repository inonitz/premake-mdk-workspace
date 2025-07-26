#include "internal_input.hpp"
#include "awc2/input_types.hpp"
#include <util2/C/marker4.h>
#include <GLFW/glfw3.h>


namespace AWC2::internal {


using istate = AWC2::Input::inputState;
using ikey   = AWC2::Input::keyCode;
using ibutton = AWC2::Input::mouseButton;
using icurpos = AWC2::Input::cursorPosition;


void InputState::reset()
{
	for(u8 i = 0; i < sizeof(keyboardKeys) / sizeof(keyboardKeys[0]); ++i) {
		keyboardKeys[i] = 0x00;
	}
	mouseMovedFlag[0] = mouseMovedFlag[1];
	mouseMovedFlag[1] = false;
	scrollMovedFlag[0] = scrollMovedFlag[1];
	scrollMovedFlag[1] = false;
	return;
}


istate InputState::getKeyState(ikey key)
{
    return __scast(istate, keyboardKeys[__scast(u8, key)]);
}

void InputState::setKeyState(ikey key, u8 state)
{
    keyboardKeys[__scast(u8, key)] = state;
    return;
}

void InputState::updateMousePosition(icurpos const& newPosition)
{
    previousFramePos = currentFramePos;
    currentFramePos = newPosition;
    mouseMovedFlag[1] = true;
}

void InputState::updateScrollOffset(icurpos const& newOffset)
{
    previousFrameScroll = currentFrameScroll;
    currentFrameScroll = newOffset;
    scrollMovedFlag[1] = true;
}

istate InputState::getMouseButtonState(ibutton button)
{
    return __scast(istate, mouseButtons[__scast(u8, button)]);
}

void InputState::setMouseButtonState(ibutton button, u8 state)
{
    mouseButtons[__scast(u8, button)] = state;
    return;
}


static constexpr u16 global_glfwKeys[__scast(u8, ikey::ENUM_MAX)] = { 
	GLFW_KEY_ESCAPE,
	GLFW_KEY_SPACE,
	GLFW_KEY_0,
	GLFW_KEY_1,
	GLFW_KEY_2,
	GLFW_KEY_3,
	GLFW_KEY_4,
	GLFW_KEY_5,
	GLFW_KEY_6,
	GLFW_KEY_7,
	GLFW_KEY_8,
	GLFW_KEY_9,
	GLFW_KEY_W,
	GLFW_KEY_A,
	GLFW_KEY_S,
	GLFW_KEY_D,
	GLFW_KEY_Q,
	GLFW_KEY_E,
	GLFW_KEY_R,
	GLFW_KEY_F,
	GLFW_KEY_C,
	GLFW_KEY_X,
	GLFW_KEY_Z,
	GLFW_KEY_T,
	GLFW_KEY_G,
	GLFW_KEY_V,
	GLFW_KEY_B,
	GLFW_KEY_H,
	GLFW_KEY_Y,
	GLFW_KEY_U,
	GLFW_KEY_J,
	GLFW_KEY_N,
	GLFW_KEY_M,
	GLFW_KEY_K,
	GLFW_KEY_I,
	GLFW_KEY_O,
	GLFW_KEY_L,
	GLFW_KEY_P
};


static constexpr const char* global_keyNames[__scast(u8, ikey::ENUM_MAX) + 1] = { 
	"KEY_ESCAPE",
	"KEY_SPACE",
	"KEY_0",
	"KEY_1",
	"KEY_2",
	"KEY_3",
	"KEY_4",
	"KEY_5",
	"KEY_6",
	"KEY_7",
	"KEY_8",
	"KEY_9",
	"KEY_W",
	"KEY_A",
	"KEY_S",
	"KEY_D",
	"KEY_Q",
	"KEY_E",
	"KEY_R",
	"KEY_F",
	"KEY_C",
	"KEY_X",
	"KEY_Z",
	"KEY_T",
	"KEY_G",
	"KEY_V",
	"KEY_B",
	"KEY_H",
	"KEY_Y",
	"KEY_U",
	"KEY_J",
	"KEY_N",
	"KEY_M",
	"KEY_K",
	"KEY_I",
	"KEY_O",
	"KEY_L",
	"KEY_P",
	"KEY_UNKOWN"
};


static constexpr u16 global_glfwMouseButtons[__scast(u8, ibutton::ENUM_MAX)] = { 
	GLFW_MOUSE_BUTTON_LEFT,
	GLFW_MOUSE_BUTTON_RIGHT, 
	GLFW_MOUSE_BUTTON_MIDDLE 
};




u16 toGLFWKeyCode(ikey kc)
{
	return global_glfwKeys[static_cast<u8>(kc)];
}

ikey toKeyCode(u16 glfw)
{
	u8 i = 0;
	while( i < (u8)ikey::ENUM_MAX && glfw != global_glfwKeys[i]) { ++i; }
	util2_debugnobr(if(unlikely(i == (u8)ikey::ENUM_MAX)) {
		markfmt("glfwKeyToKeyCode() ==> couldn't find glfw-keyCode of value %u\n", glfw);
	});
	return __scast(ikey, i);
}

const char* keyCodeToString(ikey kc) 
{
	return global_keyNames[__scast(u8, kc)];
}

u16 toGLFWMouseButton(ibutton button)
{
	return global_glfwMouseButtons[__scast(u8, button)];
}

ibutton toMouseButton(u16 glfw)
{
	u8 i = 0;
	while( i < (u8)ibutton::ENUM_MAX && glfw != global_glfwMouseButtons[i]) { ++i; }
	util2_debugnobr(if(unlikely(i == (u8)ibutton::ENUM_MAX)) {
		markfmt("glfwMouseButtonToButton() ==> couldn't find glfw-button of value %u\n", glfw);
	});
	return __scast(ibutton, i);
}


} // namespace AWC2::internal