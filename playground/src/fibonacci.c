#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

// #define UNUSED(_parameter) (void)(_parameter)

int fibonacci(int x)
{
	if (x == 0)
		return 0;
	else if (x == 1)
		return 1;

	return fibonacci(x - 1) + fibonacci(x - 2);
}

//--------------------------------------------------------------------- MAIN ---
int main(int argc, char const *argv[])
{
	int rounds;
	if (argc > 1) {
		// rounds = atoi(argv[1]);
		sscanf(argv[1], "%d", &rounds);
	}
	else {
		rounds = 10;
	}

	printf("%d rounds\n", rounds);

	for (int i = 0; i < rounds; i++) {
		printf("%d\n", fibonacci(i));
	}

	return 0;
}