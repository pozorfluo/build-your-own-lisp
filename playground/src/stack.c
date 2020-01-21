#include <stdint.h>
#include <stdio.h>

//------------------------------------------------------------------- MACROS ---
// see
// https://stackoverflow.com/questions/45220134/how-to-cast-integer-value-to-pointer-address-without-triggering-warnings
#define DUMP_INTPTR(_pointer)                                                  \
	printf(#_pointer "      \t %p : %d : %p\n",                                \
	       (void *)_pointer,                                                   \
	       *_pointer,                                                          \
	       (void *)((uint32_t *)(uintptr_t)(*_pointer)));

//--------------------------------------------------------------------- MAIN ---
// TODO pay attention to alignment
//     (&arg2 + i)              0x7ffc12f412f4 : 32764 : 0x7ffc
//     (arg3_ptr + i)           0x7ffc12f412f4 : 32764 : 0x7ffc
//     (ptrptr + i)             0x7ffc12f412f4 : 32764 : 0x7ffc
//     (&arg2 + i)              0x7ffc12f412f8 : 542094552 : 0x204fb4d8
//     (arg3_ptr + i)           0x7ffc12f412f8 : 542094552 : 0x204fb4d8

int call(int arg1, int arg2, int *arg3_ptr)
{
	for (int i = -8; i < 20; i++) {
		DUMP_INTPTR((arg3_ptr + i));
	}
	uint32_t *ptrptr = (uint32_t *)(uintptr_t)(&arg1);
	arg3_ptr      = &arg1;
	for (int i = -8; i < 20; i++) {
		DUMP_INTPTR((&arg1 + i));
		DUMP_INTPTR((arg3_ptr + i));
		DUMP_INTPTR((ptrptr + i));
	}

	ptrptr = (uint32_t *)(uintptr_t)(&arg2);
	arg3_ptr      = &arg2;
	for (int i = -8; i < 20; i++) {
		DUMP_INTPTR((&arg2 + i));
		DUMP_INTPTR((arg3_ptr + i));
		DUMP_INTPTR((ptrptr + i));
	}

	return 99;
}
//--------------------------------------------------------------------- MAIN ---
int main(void)
{
	int a       = 7;
	int array[] = {6, 6, 6, 6, 6, 6};
	int b       = 8;
	int *ptr    = array;

	DUMP_INTPTR(&a);
	DUMP_INTPTR(&b);
	DUMP_INTPTR(array);

	for (int i = -8; i < 20; i++) {
		DUMP_INTPTR((ptr + i));
	}

	call(90, 91, array);
	return 0;
}
