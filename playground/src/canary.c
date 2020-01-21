/**
 * compile with
 *   gcc -S -o canary_no-ssp.s -fno-stack-protector canary.c
 *   gcc -S -o canary_ssp.s -fstack-protector canary.c
 *
 * compare generated assembly code
 *
 * debug
 *   gdb -q ../../bin/canary
 *   list
 *   break 13
 *   run 'argsdjflsdlfkljklsdjfksdlfj'
 *   step
 */

#include <stdio.h>
#include <string.h>

void vulnerable_function(const char *string)
{
	char buffer[20];
	strcpy(buffer, string);
}

int call_me_from_gdb(int a, int b)
{
	int c;
	c = a + 5;
	c += b;
	printf("\n%d\n", c);
	return c;
}

void hello(void) { printf("hello .gdbinit !\n"); }

int main(int argc, char const *argv[])
{
	int a = argc;
	int b = 8;
	printf("\n%d %d %s\n", a, b, argv[1]);
	vulnerable_function(argv[1]);
	return 0;
}
