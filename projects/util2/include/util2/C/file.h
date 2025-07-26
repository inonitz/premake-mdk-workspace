#ifndef __UTIL2_C_DEFINITION_LOAD_FILE__
#define __UTIL2_C_DEFINITION_LOAD_FILE__
#include "util2/C/util2_api.h"
#include "util2/C/util2_extern.h"
#include "util2/C/base_type.h"
#include "util2_api.h"


UTIL2_EXTERNC_DECL_BEGIN


/* 
	Recommended to set 'size'=0, 'out'=NULL to find the size of the buffer that should be allocated 
	(if you didn't know the filesize beforehand)
	when 'size' is known, allocate a buffer and pass it in 'out', 
	After which the function will proceed with file loading
*/
UTIL2_API bool_t util2_load_file(
	const char* path, /* filepath 													  */ 
	u32* 	    size, /* the size of the out array that was pre-allocated.            */
	char*		out   /* Where to place the file contents (Buffer of Min-Size 'size') */
);


/*
	pass 'size'=0 to know how much memory to allocate for the 'out' string
	if 'size' of 'out' is too small then 'size' will be updated
*/
UTIL2_API UTIL2_DEPRECATED_MSG("NOT IMPLEMENTED FOR C") void util2_current_path(
	u32*  size,
	char* out
);


UTIL2_EXTERNC_DECL_END


#endif /* __UTIL2_C_DEFINITION_LOAD_FILE__ */