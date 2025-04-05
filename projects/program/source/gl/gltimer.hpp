#ifndef __OPENGL_OBJECTS_TIMER__
#define __OPENGL_OBJECTS_TIMER__
#include <util/base_type.h>


/* Implemented with help from https://www.lighthouse3d.com/tutorials/opengl-timer-query/ */
namespace Time {


struct GPUTimer 
{
public:
	using type_t = GPUTimer;


	void create();
	void destroy();
	static void begin(type_t& timer);
	static void end(type_t& timer);

	/* after calling GPUTimer::end(), calling currentFrame() might not have a value yet */
	u64 currentFrame()  const { return m_query_result[1]; }
	u64 previousFrame() const { return m_query_result[0]; }
private:
	u32 m_query[2];
	u64 m_query_result[2];
};


} /* namespace Timer */


#endif