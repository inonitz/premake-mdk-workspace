#ifndef __AWC2_INTERNAL_DEFAULT_CALLBACK_DEFINITION_HEADER__
#define __AWC2_INTERNAL_DEFAULT_CALLBACK_DEFINITION_HEADER__
#include <util2/C/macro.h>


typedef struct GLFWwindow GLFWwindow;
typedef struct GLFWmonitor GLFWmonitor;

namespace AWC2::internal {


void glfw_framebuffer_size_callback(
	GLFWwindow* handle,
	int w, 
	int h
);
void glfw_key_callback(
	GLFWwindow* handle,
	int key,
	notused int scancode,
	int action,
	notused int mods
);
void glfw_window_focus_callback(
	GLFWwindow* window,
	int 		focused
);
void glfw_cursor_position_callback(
	GLFWwindow* window,
	double xpos,
	double ypos
);
void glfw_scroll_offset_callback(
	GLFWwindow* window,
	double xoffset,
	double yoffset
);
void glfw_mouse_button_callback(
	GLFWwindow* window,
	int button, 
	int action, 
	notused int mods
);
void glfw_window_close_callback(
	GLFWwindow* window
);
void glfw_mouse_cursor_enter_callback(
	GLFWwindow* window,
	int entered
);
void glfw_character_callback(
	GLFWwindow* window,
	unsigned int ch
);
void glfw_monitor_callback(
	GLFWmonitor* monitor,
	int event
);
void gl_debug_message_callback(
	unsigned int 		source, 
	unsigned int 		type, 
	unsigned int 		id, 
	unsigned int 		severity, 
	notused signed int  length, 
	char const*         message, 
    notused void const* user_param
);

/* Should probably be defined in a different file (polluting the namespace with unused funcs), but this works now */
struct alignsz(64) glfw_callback_table {	
	decltype(glfw_framebuffer_size_callback)*   framebuffer_size = glfw_framebuffer_size_callback;
	decltype(glfw_key_callback)* 		 	    key 			 = glfw_key_callback;
	decltype(glfw_window_focus_callback) *	    window_focus 	 = glfw_window_focus_callback;
	decltype(glfw_cursor_position_callback)*    cursor_position  = glfw_cursor_position_callback;
	decltype(glfw_scroll_offset_callback)*      scroll_offset    = glfw_scroll_offset_callback;
	decltype(glfw_mouse_button_callback)*       mouse_button 	 = glfw_mouse_button_callback;
	decltype(glfw_window_close_callback)*       close_window 	 = glfw_window_close_callback;
	decltype(glfw_mouse_cursor_enter_callback)* mouse_enter  	 = glfw_mouse_cursor_enter_callback;
	decltype(glfw_character_callback)*  		text_input 		 = glfw_character_callback;
	decltype(glfw_monitor_callback)*  			monitor_events   = glfw_monitor_callback;
	void* reserved[6]{nullptr};
};


} // namespace AWC2::internal


#endif