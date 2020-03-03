//------------------------------------------------------------------ SUMMARY ---
/**
 * Help follow malloc / free where needed
 */

#ifndef debug_xmalloc_h
#define debug_xmalloc_h
//------------------------------------------------------------- DEBUG MACROS ---

#ifdef DEBUG_MALLOC

#include <malloc.h>

void *xmalloc(size_t size, const char *origin, const char *destination)
{
	void *return_pointer = malloc(size);
	if ((return_pointer == NULL) && !size) {
		return_pointer = malloc(1);
	}
	if ((return_pointer == NULL)) {
		printf(FG_RED REVERSE
		       "malloc failed, Out of memory I guess\n"
		       "\tcalled inside      : %s()\n"
		       "\ttried to allocate  : %lu bytes\n"
		       "\tfor                : %s\n" RESET,
		       origin,
		       size,
		       destination);
		exit(EXIT_FAILURE);
	}

	printf(FG_CYAN "\tmallocing %s %lu bytes inside %s()\n" RESET,
	       destination,
	       size,
	       origin);
	memset(return_pointer, 0xFB, size);
	return return_pointer;
}

void xfree(void *pointer, const char *pointer_name, const char *origin)
{
	size_t bytes_in_malloced_block = malloc_usable_size(pointer);
	printf(FG_BLUE "\tfreeing %s %lu bytes inside %s()\n" RESET,
	       pointer_name,
	       bytes_in_malloced_block,
	       origin);
	free(pointer);
}

#define XMALLOC(_size, _origin, _destination)                                  \
	xmalloc(_size, _origin, _destination)

#define XFREE(_pointer, _origin) xfree(_pointer, #_pointer, _origin)

#else
#define XMALLOC(_size, _origin, _destination) malloc(_size)
#define XFREE(_pointer, _origin) free(_pointer)
#endif

#endif // debug_xmalloc_h	