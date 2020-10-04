#include <stdio.h>
#include <string.h>

/**
 * Reinterpret cast through a union macro.
 *
 * note Is bad juju, using the apparently idiomatic memcpy way.
 *
 * Similarly, access by taking the address, casting the resulting pointer and
 *  dereferencing the result has undefined behavior, even if the cast uses a
 *  union type...
 * see https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html
 * 
 * see https://gcc.gnu.org/onlinedocs/gcc/Typeof.html
 */

#define UNION_CAST(_src, _destination_type)                                    \
	(((union {                                                                 \
		 __typeof__(_src) a;                                                   \
		 _destination_type b;                                                  \
	 } *)&_src)                                                                \
	     ->b)

//--------------------------------------------------------------------- MAIN ---
/**
 *
 */
int main(void)
{

	puts("UNION_CAST (bad juju)");
	float my_float = 3.6;
	int my_int     = UNION_CAST(my_float, int);

	printf("my_float : %f | my_int : %d\n", my_float, my_int);

	puts("memcpy");
	memcpy(&my_int, &my_float, sizeof(my_float));
	printf("my_float : %f | my_int : %d\n", my_float, my_int);
	return 0;
}