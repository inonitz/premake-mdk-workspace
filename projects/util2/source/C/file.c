#include "util2/C/file.h"
#include "util2/C/ifcrash2.h"
#include "util2/C/macro.h"
#include <stdio.h>
#include <inttypes.h>
#include <errno.h>


UTIL2_EXTERNC_DECL_BEGIN


bool_t util2_load_file(
	const char* path,
	u32* 	    size,
	char*		out
) {
	u64 fsize;
	FILE* to_open = NULL;

	to_open = fopen(path, "rb");
	ifcrashfmt(!to_open, "Couldn't get handle to file at Path [%s]\n", path);
	fseek(to_open, 0, SEEK_END);
	fsize = ftell(to_open);
	fseek(to_open, 0, SEEK_SET);

	if(*size < fsize || out == NULL) {
		/* A buffer wasn't allocated OR the size of the buffer is too small. */
		*size = fsize;
		fclose(to_open);
		return BOOL_FALSE;
	}


	fsize = fread(out, sizeof(byte), *size, to_open);
	ifcrashdo(fsize != *size, {
		fclose(to_open); 
		printf("Something went wrong - file size is %" PRIu64 " bytes, read only %" PRIu64 " bytes\n", __scast(u64, *size), fsize);
	});


	fsize = fclose(to_open);
	ifcrashfmt(fsize, "Couldn't close file handle. ERROR CODE: %" PRId64 "\n", fsize, errno);
	return BOOL_TRUE;
}


void util2_current_path(
	notused u32*  size,
	notused char* out
) {
    return;
}


UTIL2_EXTERNC_DECL_END