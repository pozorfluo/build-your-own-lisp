#include <limits.h>
#include <stdio.h>

//--------------------------------------------------------------------- MAIN ---
int main(void)
{
	printf(
	    "Char is guaranteed to be 1 byte.\n"
	    "But a byte is NOT guaranteed to be 8 bits.\n\n"
	    "On this system byte size is : %d bits\n\n",
	    CHAR_BIT);
	return 0;
}
