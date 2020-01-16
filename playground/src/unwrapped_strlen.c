
#include <stdio.h>

#include <stdint.h>

/**
 * see https://stackoverflow.com/questions/34642994/understanding-code-in-strlen-implementation/34643025
 * 
 */

#define LONGPTR_MASK (sizeof(long) - 1)
// LONG_BIT == 64
static const unsigned long mask01 = 0x0101010101010101;
static const unsigned long mask80 = 0x8080808080808080;

void print_bits(unsigned long number, unsigned int bit_width)
{
	if (number || (bit_width > 0)) {
		print_bits(number >> 1, bit_width - 1);
		printf("%d", (int)(number & 1));

		if (!(bit_width % 4)) {
			putchar(' ');
		}
	}
}

size_t playground_strlen(const char *string)
{
	const char *		 p;
	const unsigned long *lp;
	long				 va, vb;

	printf("str = %s\n", string);

	puts("\n");


	unsigned long longptr_mask = 7;
	printf("longptr_mask = %lx\n", longptr_mask);
	print_bits(longptr_mask, 64);
	puts("\n");

	unsigned long not_longptr_mask = ~longptr_mask;
	printf("not_longptr_mask = %lx\n", not_longptr_mask);
	print_bits(not_longptr_mask, 64);
	puts("\n");

	lp = (const unsigned long *)((uintptr_t)string & ~LONGPTR_MASK);

	printf("str = %p\n", (void *)string);
	print_bits((unsigned long)string, 64);
	puts("\n");

	printf("lp  = %p\n", (void *)lp);
	print_bits((unsigned long)lp, 64);
	puts("\n");
	
	printf("*lp  = %lx\n", *lp);
	print_bits(*lp, 64);
	puts("\n");


	va = (*lp - mask01);
	vb = ((~*lp) & mask80);

	printf("va = %ld\n", va);
	printf("vb = %ld\n", vb);

	lp++;
	if (va & vb)
		/* Check if we have \0 in the first part */
		for (p = string; p < (const char *)lp; p++)
			if (*p == '\0')
				return (p - string);

	/* Scan the rest of the string using word sized operation */
	for (;; lp++) {
		va = (*lp - mask01);
		vb = ((~*lp) & mask80);
		if (va & vb) {
			p = (const char *)(lp);
			if (p[0] == '\0')
				return (p - string);

			if (p[1] == '\0')
				return (p - string + 1);

			if (p[2] == '\0')
				return (p - string + 2);

			if (p[3] == '\0')
				return (p - string + 3);

			if (p[4] == '\0')
				return (p - string + 4);

			if (p[5] == '\0')
				return (p - string + 5);

			if (p[6] == '\0')
				return (p - string + 6);

			if (p[7] == '\0')
				return (p - string + 7);
		}
	}

	/* NOTREACHED */
	return (0);
}

int main()
{
	printf("%zu\n", (sizeof(long) - 1));
	printf("%zu\n", mask01);
	printf("%zu\n", mask80);
	printf("%zu\n", playground_strlen("mask80"));

	return 0;
}
