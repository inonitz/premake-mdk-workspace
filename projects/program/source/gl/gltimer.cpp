#include "gltimer.hpp"
#include <glbinding/gl/functions.h>
#include <glbinding/gl/enum.h>


void Time::GPUTimer::create()
{
    // gl::glCreateQueries(gl::GL_TIMESTAMP, 2, &m_query[0]);
    gl::glGenQueries(2, &m_query[0]);
    return;
}


void Time::GPUTimer::destroy()
{
    gl::glDeleteQueries(2, &m_query[0]);
    return;
}


void Time::GPUTimer::begin(type_t& timer)
{
    /* Place Resulted Query in query[0] 'backbuffer' */
    gl::glBeginQuery(gl::GL_TIME_ELAPSED, timer.m_query[0]);
    return;
}


void Time::GPUTimer::end(type_t& timer)
{
    gl::glEndQuery(gl::GL_TIME_ELAPSED);

    /* update the result from the previous frame */
    timer.m_query_result[0] = timer.m_query_result[1];
    gl::glGetQueryObjectui64v(
        timer.m_query[0], 
        gl::GL_QUERY_RESULT, 
        &timer.m_query_result[1]
    );

    std::swap(timer.m_query[0], timer.m_query[1]);
    return;
}



/* 
    int done = 0;
    while(!done) {
        glGetQueryObjectiv(query, GL_QUERY_RESULT_AVAILABLE, &done);
    }
    [NOTE]:
    we're not waiting for a query to be available for performance sake =>
    calling GL_QUERY_RESULT_AVAILABLE flushes the pipeline & waits for everything to sync,
    which is probably not good (lol)
    Therefore, we'll query multiple objects every frame
    such that a result will MOST LIKELY be available
*/