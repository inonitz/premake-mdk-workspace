#include "internal_callback.hpp"
#include "awc2/context.hpp"
#include "internal_event.hpp"
#include "internal_instance.hpp"
#include "internal_state.hpp"


#include <util2/C/marker4.h>
#ifdef __linux__ /* Happens to be that glext.h has snuck into my codebase so I disable it */
#	define GL_GLEXT_LEGACY /* https://fsunuc.physics.fsu.edu/git/gwm17/glfw/commit/8db1528c7496bb36884ee93791091d5eb186e6cd */
#endif
#include <glbinding/gl/gl.h>
#include <imgui/imgui_impl_glfw.h>
#include <GLFW/glfw3.h>


#define __call_user_callback_func(func_type, __args) __rcast(func_type, \
			__awc2_lib_get_active_context()->m_event_table.pointers[UserFuncIndexer<func_type>()()] \
		)(&__args);

#define __call_user_callback_func2(func_type, context_id, __args_ptr) \
	func_type __local_func_ptr = __rcast(func_type, \
		__awc2_lib_get_context(context_id).m_event_table.pointers[ \
			UserFuncIndexer<func_type>()() \
		]); \
	(__local_func_ptr)(__args_ptr); \
	/* (*__local_func_ptr)(__args_ptr); \ */
	\

#define __call_imgui_callback_func(context_id, func_name, ...) \
	auto* const old_ctx = ImGui::GetCurrentContext(); \
	ImGui::SetCurrentContext(__rcast(ImGuiContext*, \
		internal::__awc2_lib_get_context(context_id).m_imgui) \
	); \
	func_name(__VA_ARGS__); \
	ImGui::SetCurrentContext(old_ctx); \


namespace AWC2::internal {


void glfw_framebuffer_size_callback(notused GLFWwindow* handle, i32 w, i32 h) 
{
	if(w < 0 || h < 0)
		markfmt("Width/Height in invalid ranges (%d x %d)", w, h);


	const auto active_ctxt_id = *__scast(u8*, glfwGetWindowUserPointer(handle));
	auto& win_data = __awc2_lib_get_context(active_ctxt_id).m_window.m_data;


	__awc2_lib_get_context(active_ctxt_id).setCurrent();
	gl::glViewport(0, 0, w, h);
    bool minimized = ( (w == 0) || (h == 0) );
    bool sizeChange = !minimized && ( 
			win_data.width  != __scast(u16, w) || 
			win_data.height != __scast(u16, h) 
		);
    

    win_data.description.stateFlags &= ~(WindowStateFlag::MINIMIZED | WindowStateFlag::SIZE_CHANGED);
    win_data.description.stateFlags |= (
        from_conditional(WindowStateFlag::SIZE_CHANGED, sizeChange) 
        | 
        from_conditional(WindowStateFlag::MINIMIZED, minimized)
    );
	user_callback_winsize_struct __funcargs{
		handle, 
		active_ctxt_id,
		{0},
		__scast(u16, w), 
		__scast(u16, h)
	};
	__call_user_callback_func2(user_callback_window_size, active_ctxt_id, &__funcargs);


	markfmt("[[%02hhu][framebuffer_callback][Before=%ux%i]  Window Size Changed  [After=%ux%u]",
		active_ctxt_id,
		win_data.width, 
		win_data.height,
		w, h
	);
	// markfmt("[framebuffer_callback][%u][new_size = %u x %u]", active_ctxt_id, w, h);
	win_data.width  = __scast(u16, w);
	win_data.height = __scast(u16, h);
	return;
}


void glfw_key_callback(
	notused GLFWwindow* handle,
	int key, 
	int scancode, 
	int action, 
	int mods
) {
	static std::array<const char*, (u8)Input::inputState::ENUM_MAX + 1> actionStr = {
		"RELEASED",
		"PRESSED ",
		"REPEAT  ",
		""
	};
	

	/* Updating the state of the m_io struct */
	const auto active_ctxt_id = *__rcast(u8*, glfwGetWindowUserPointer(handle));
    auto& key_state = __awc2_lib_get_context(active_ctxt_id).m_io;
	Input::keyCode keyCodeIndex = AWC2::internal::toKeyCode(key);
	__release_unused u8 before = __scast(u8,
		key_state.getKeyState(keyCodeIndex)
	);
	actionStr[3] = actionStr[static_cast<u8>(action)];
	key_state.setKeyState(keyCodeIndex, (1 << action));


	/* Calling the user callback */
	user_callback_keyboard_struct __funcargs{
		handle,
		active_ctxt_id,
		keyCodeIndex, 
		__scast(Input::inputState, (1 << action) ),
		{0},
	};

	user_callback_keyboard __local_func_ptr = 
		reinterpret_cast<user_callback_keyboard>(
			__awc2_lib_get_context(active_ctxt_id).m_event_table.pointers[UserFuncIndexer<user_callback_keyboard>()()]
		);
	__local_func_ptr(&__funcargs);
	// __call_user_callback_func2(user_callback_keyboard, active_ctxt_id, &__funcargs);


	/* Call the ImGui Context Callback */
	__call_imgui_callback_func(active_ctxt_id, ImGui_ImplGlfw_KeyCallback, 
		handle, key, scancode, action, mods
	);


	/* printing in debug mode */
	__release_unused const char* key_name = glfwGetKeyName(key, scancode);
	key_name = (key_name == nullptr) ? AWC2::internal::keyCodeToString(keyCodeIndex) : key_name;
	markfmt("[[%02hhu]key_callback][kci=%02hhu][Before=%u]  [%s]  Key %s  [After=%u]", 
		active_ctxt_id,
		__scast(u8, keyCodeIndex),
		before,
		actionStr[3],
		key_name, 
		__scast(u8, key_state.getKeyState(keyCodeIndex) )
	);
	// markfmt("[key_callback][%u][%s][%s]", active_ctxt_id, key_name, actionStr[3]);
	return;
}


void glfw_window_focus_callback(
	notused GLFWwindow* window,
	int 				focused
) {
	const auto active_ctxt_id = *__rcast(u8*, glfwGetWindowUserPointer(window));
	auto& win_data = __awc2_lib_get_context(active_ctxt_id).m_window.m_data;
	__release_unused static const std::array<const char*, 4> actionStr = {
		"UNFOCUSED",
		"FOCUSED  ",
		"Unfocused",
		"Focused  "
	};
	__release_unused u8 before = __scast(bool, win_data.description.stateFlags & WindowStateFlag::FOCUSED);
	__release_unused u8 after  = boolean(focused);


    win_data.description.stateFlags &= ~WindowStateFlag::FOCUSED;
    win_data.description.stateFlags |= from_conditional(WindowStateFlag::FOCUSED, focused);
    user_callback_winfocus_struct __funcargs{
		window, 
		active_ctxt_id,
		__scast(bool, focused),
		{0} 
	};
	__call_user_callback_func2(user_callback_window_focus, active_ctxt_id, &__funcargs);


	__call_imgui_callback_func(active_ctxt_id, ImGui_ImplGlfw_WindowFocusCallback,
		window, focused
	);


	markfmt("[[%02hhu]window_focus_callback][fi=%02hhu][Before=%u]  [%s]  Window %s  [After=%u]",
		active_ctxt_id,
		__scast(u8, focused),
		before,
		actionStr[after],
		actionStr[after + 2],
		after
	);
	// markfmt("[focus_callback][%u][%s]", active_ctxt_id, actionStr[after]);
	return;
}


void glfw_cursor_position_callback(
	notused GLFWwindow* window,
	double xpos, 
	double ypos
) {
	const auto active_ctxt_id = *__rcast(u8*, glfwGetWindowUserPointer(window));
	auto& mouse_state = __awc2_lib_get_context(active_ctxt_id).m_io;
    mouse_state.updateMousePosition(Input::cursorPosition{{{ 
		__scast(f32, xpos), 
		__scast(f32, ypos) 
	}}});


	user_callback_mousecursor_struct __funcargs{
		window, 
		active_ctxt_id,
		{0},
		{{{xpos, ypos}}} 
	};
	__call_user_callback_func2(user_callback_mouse_pos, active_ctxt_id, &__funcargs);
	

	__call_imgui_callback_func(active_ctxt_id, ImGui_ImplGlfw_CursorPosCallback,
		window, xpos, ypos
	);	
	return;
}


void glfw_scroll_offset_callback(
	notused GLFWwindow* window,
	double xoffset,
	double yoffset
) {
	const auto active_ctxt_id = *__rcast(u8*, glfwGetWindowUserPointer(window));
	auto& mouse_state = __awc2_lib_get_context(active_ctxt_id).m_io;
	mouse_state.updateScrollOffset(Input::cursorPosition{{{ 
		__scast(f32, xoffset), 
		__scast(f32, yoffset) 
	}}});


	user_callback_mousescroll_struct __funcargs{
		window, 
		active_ctxt_id,
		{0},
		{{{ xoffset, yoffset }}}
	};
	__call_user_callback_func2(user_callback_mouse_scroll, active_ctxt_id, &__funcargs);


	__call_imgui_callback_func(active_ctxt_id, ImGui_ImplGlfw_ScrollCallback,
		window, xoffset, yoffset
	);	
	return;	
}


void glfw_mouse_button_callback(
	notused GLFWwindow* window,
	int button, 
	int action, 
	int mods
) {
	static std::array<const char*, __scast(u8, Input::mouseButton::ENUM_MAX) + 1> actionStr = {
		"RELEASED",
		"PRESSED ",
		"REPEAT  ",
		""
	};
	static std::array<const char*, __scast(u8, Input::mouseButton::ENUM_MAX) + 2> ButtonNames = {
		"MOUSE_BUTTON_LEFT  ",
		"MOUSE_BUTTON_RIGHT ",
		"MOUSE_BUTTON_MIDDLE",
		"MOUSE_BUTTON_UNKOWN",
		""
	};


	const auto active_ctxt_id = *__rcast(u8*, glfwGetWindowUserPointer(window));
	auto& mouse_state = __awc2_lib_get_context(active_ctxt_id).m_io;
    Input::mouseButton buttonIndex = AWC2::internal::toMouseButton(button); /* might return MoueButton::ENUM_MAX */
	__release_unused u8 before = __scast(u8,
		mouse_state.getMouseButtonState(buttonIndex)
	);
	actionStr[3]   = actionStr[static_cast<u8>(action)];
	ButtonNames[4] = ButtonNames[static_cast<u8>(buttonIndex)];
	mouse_state.setMouseButtonState(buttonIndex, (1 << action));


	user_callback_mousebutton_struct __funcargs{
		window, 
		active_ctxt_id,
		buttonIndex, __scast(Input::inputState, (1 << action) ),
		{0} 
	};
	__call_user_callback_func2(user_callback_mouse_button, active_ctxt_id, &__funcargs);


	__call_imgui_callback_func(active_ctxt_id, ImGui_ImplGlfw_MouseButtonCallback,
		window, button, action, mods
	);


	markfmt("[[%02hhu]mouse_button_callback][bi=%02hhu][Before=%u]  [%s]  Mouse Button %s  [After=%u]",
		active_ctxt_id, 
		__scast(u8, buttonIndex),
		before,
		ButtonNames[4],
		actionStr[3],
		__scast(u8, mouse_state.getMouseButtonState(buttonIndex) )
	);
	// markfmt("[mouse_button_callback][%u][%s][%s]", active_ctxt_id, ButtonNames[4], actionStr[3]);
	return;  
}


void glfw_window_close_callback(GLFWwindow* window)
{
	const auto active_ctxt_id = *__rcast(u8*, glfwGetWindowUserPointer(window));
	AWC2::closeContext(active_ctxt_id);
	markfmt("[window_close_callback][%02hhu]", active_ctxt_id);
	return;
}


void glfw_mouse_cursor_enter_callback(
	GLFWwindow* window,
	int entered
) {
	const auto active_ctxt_id = *__rcast(u8*, glfwGetWindowUserPointer(window));	
	

	__call_imgui_callback_func(active_ctxt_id, ImGui_ImplGlfw_CursorEnterCallback,
		window, entered
	);
	return;
}


void glfw_character_callback(
	GLFWwindow* window,
	unsigned int ch
) {
	const auto active_ctxt_id = *__rcast(u8*, glfwGetWindowUserPointer(window));


	__call_imgui_callback_func(active_ctxt_id, ImGui_ImplGlfw_CharCallback, 
		window, ch
	);
	return;
}


void glfw_monitor_callback(
	GLFWmonitor* monitor,
	int event
) {
	for(auto& ctx : AWC2::getActiveContextList())
	{
		AWC2::setCurrentContext(ctx);
		/* Incase every context has a different callback for some reason (?) */

		ImGui_ImplGlfw_MonitorCallback(monitor, event);

	}
	AWC2::setCurrentContext();
	return;
}




#ifdef _DEBUG
void gl_debug_message_callback(
	uint32_t 		  	source, 
	uint32_t 		  	type, 
	uint32_t 		  	id, 
	uint32_t 		  	severity, 
	notused int32_t     length, 
	char const*         message, 
	notused void const* user_param
) {
	const std::pair<gl::GLenum, const char*> srcStr[6] = {
		{ gl::GLenum::GL_DEBUG_SOURCE_API,             "API" 			 },
		{ gl::GLenum::GL_DEBUG_SOURCE_WINDOW_SYSTEM,   "WINDOW SYSTEM"   },
		{ gl::GLenum::GL_DEBUG_SOURCE_SHADER_COMPILER, "SHADER COMPILER" },
		{ gl::GLenum::GL_DEBUG_SOURCE_THIRD_PARTY,	   "THIRD PARTY" 	 },
		{ gl::GLenum::GL_DEBUG_SOURCE_APPLICATION,	   "APPLICATION" 	 },
		{ gl::GLenum::GL_DEBUG_SOURCE_OTHER, 		   "OTHER" 			 }
	};
	const std::pair<gl::GLenum, const char*> typeStr[7] = {
		{ gl::GLenum::GL_DEBUG_TYPE_ERROR, 			     "ERROR"               },
		{ gl::GLenum::GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, "DEPRECATED_BEHAVIOR" },
		{ gl::GLenum::GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR,  "UNDEFINED_BEHAVIOR"  },
		{ gl::GLenum::GL_DEBUG_TYPE_PORTABILITY,		 "PORTABILITY" 	       },
		{ gl::GLenum::GL_DEBUG_TYPE_PERFORMANCE,		 "PERFORMANCE" 		   },
		{ gl::GLenum::GL_DEBUG_TYPE_MARKER,			   	 "MARKER" 			   },
		{ gl::GLenum::GL_DEBUG_TYPE_OTHER,			     "OTHER" 			   }
	};
	const std::pair<gl::GLenum, const char*> severityStr[6] = {
		{ gl::GLenum::GL_DEBUG_SEVERITY_NOTIFICATION, "NOTIFICATION" },
		{ gl::GLenum::GL_DEBUG_SEVERITY_LOW, 		  "LOW"		     },
		{ gl::GLenum::GL_DEBUG_SEVERITY_MEDIUM, 	  "MEDIUM"	     },
		{ gl::GLenum::GL_DEBUG_SEVERITY_HIGH, 		  "HIGH"	     }
	};
	const char* src_str      = srcStr[0].second;
	const char* type_str     = typeStr[0].second;
	const char* severity_str = severityStr[0].second;
	u32 		idx 		 = 0;
	

	while(srcStr[idx].first != source) { ++idx; }
	src_str = srcStr[idx].second;
	idx = 0;

	while(typeStr[idx].first != type)  { ++idx; }
	type_str = typeStr[idx].second;
	idx = 0;

	while(severityStr[idx].first != severity)  { ++idx; }
	severity_str = severityStr[idx].second;
	idx = 0;
	
	
	printf("OPENGL >> %s::%s::%s %u: %s\n", src_str, type_str, severity_str, id, message);
	return;
}
#else
void gl_debug_message_callback(
	notused uint32_t 	source, 
	notused uint32_t 	type, 
	notused uint32_t 	id, 
	notused uint32_t 	severity, 
	notused int32_t     length, 
	notused char const* message, 
	notused void const* user_param
) {
	return;
}
#endif


} // namespace AWC::Event