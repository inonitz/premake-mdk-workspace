#ifndef __UTIL2_LOAD_FILE__
#define __UTIL2_LOAD_FILE__
#include "util2/C/util2_api.h"


namespace util2 {


/* 
	Recommended to set size{0}, out{nullptr} to find the size of the buffer that should be allocated 
	(if you didn't know the filesize beforehand)
	when 'size' is known, allocate a buffer and pass it in 'out', 
	After which the function will proceed with file loading
*/
UTIL2_API bool loadFile(
	const char*   path, /* filepath 													*/ 
	unsigned int* size, /* the size of the out array that was pre-allocated.            */
	char*		  out   /* Where to place the file contents (Buffer of Min-Size 'size') */
);


/*
	pass 'size'=0 to know how much memory to allocate for 'out'
	if 'out' is too small then 'size' will be updated
*/
UTIL2_API void current_path(
	unsigned int*  size, /* */
	char* 		   out 	 /* Where the */
);


} /* namespace util2 */


#endif /* __UTIL2_LOAD_FILE__ */