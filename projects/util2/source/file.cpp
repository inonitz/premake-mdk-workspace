#include "util2/file.hpp"
#include "util2/C/base_type.h"
#include "util2/ifcrash.hpp"
#include <filesystem>
#include <string.h>
#include <stdio.h>
#include <cinttypes>


bool util2::loadFile(
	const char*   path, /* filepath 													*/ 
	unsigned int* size, /* the size of the out array that was pre-allocated.            */
	char*		  out   /* Where to place the file contents (Buffer of Min-Size 'size') */
) {
	u64 fsize;
	FILE* to_open = nullptr;

	to_open = fopen(path, "rb");
	ifcrashfmt(!to_open, "Couldn't get handle to file at Path [%s]\n", path);
	fseek(to_open, 0, SEEK_END);
	fsize = ftell(to_open);
	fseek(to_open, 0, SEEK_SET);

	if(*size < fsize || out == nullptr) {
		/* A buffer wasn't allocated OR the size of the buffer is too small. */
		*size = fsize;
		fclose(to_open);
		return false;
	}


	fsize = fread(out, sizeof(unsigned char), *size, to_open);
	ifcrashdo(fsize != *size, {
		fclose(to_open); 
		printf("Something went wrong - file size is %" PRIu64 " bytes, read only %" PRIu64 " bytes\n", __scast(uint64_t, *size), fsize);
	});


	fsize = fclose(to_open);
	ifcrashfmt(fsize, "Couldn't close file handle. ERROR CODE: %" PRId64 "\n", fsize, errno);
	return true;
}


/* 
	Thanks to: 
	https://stackoverflow.com/questions/2868680/what-is-a-cross-platform-way-to-get-the-current-directory 
*/
void util2::current_path(
	unsigned int*  size, 
	char* 		   out
) {
	auto path     = std::filesystem::current_path().generic_u8string();
	auto pathsize = path.size();

	/* buffer too small */
	if(pathsize > *size || out == nullptr) {
		*size = pathsize;
		return;
	}
	memcpy(out, path.c_str(), pathsize);
	return;
}


void util2_current_path(
	unsigned int*  size,
	char* 		   out 	
) {
	return util2::current_path(size, out);
}