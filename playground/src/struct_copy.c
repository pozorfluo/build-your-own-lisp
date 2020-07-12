#include <stdio.h>


struct test_fsa {
	char str[20];
	int nums[10];
	int scalar;
};


//--------------------------------------------------------------------- MAIN ---
int main(void)
{
	struct test_fsa fsa = {
		"a string",
		{0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
		11
	};

	struct test_fsa copy;
	copy = fsa;

	printf(
		"fsa\n"
		"\t str[]   : %s\n"
		"\t nums[9] : %d\n"
		"\t scalar  : %d\n",
		fsa.str,
		fsa.nums[9],
		fsa.scalar
	);

	printf(
		"copy\n"
		"\t str[]   : %s\n"
		"\t nums[9] : %d\n"
		"\t scalar  : %d\n",
		copy.str,
		copy.nums[9],
		copy.scalar
	);
	
	return 0;
}