#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define BENCHMARK
/*
 * ----------------------------------------------------------- MAGIC NUMBERS ---
 */
#define TEST_COUNT 18
/*
 * ------------------------------------------------------------- DEBUG MACRO ---
 */
#ifdef BENCHMARK
#include <time.h>

#define LOOP_SIZE 10000000

#define START_BENCH(start) start = (float)clock() / CLOCKS_PER_SEC

#define STOP_BENCH(start, stop, diff, result)                                  \
	stop = (float)clock() / CLOCKS_PER_SEC;                                    \
	diff = stop - start;                                                       \
	printf("%.8f\n", diff);                                                    \
	result = diff;

#define BENCH(expression, loop, result)                                        \
	{                                                                          \
		float BENCH_start__, BENCH_end__, BENCH_diff__;                        \
		int BENCH_i__;                                                         \
		START_BENCH(BENCH_start__);                                            \
		for (BENCH_i__ = 0; BENCH_i__ < loop; BENCH_i__++) {                   \
			expression(BENCH_i__);                                             \
		}                                                                      \
		STOP_BENCH(BENCH_start__, BENCH_end__, BENCH_diff__, result);          \
	}

#else
#define LOOP_SIZE 0
#define START_BENCH(start)
#define STOP_BENCH(start, stop, diff, result)
#define BENCH(expression, loop, result)
#endif
/*
 * ---------------------------------------------------------------- Function ---
 * solves fizzbuzz style exercice for given range
 *
 *     -> void
 *
 * time  O(n)
 * space O(1)
 */
void solve_fizzbuzz(int range)
{
	/* initialize with "fizzbuzz" to avoid guessing the size ? */
	char stringToPrint[] = "fizzbuzz";

	for (int i = 1; i <= range; i++) {
		// strcpy(stringToPrint, "");
		memset(stringToPrint, '\0', sizeof(stringToPrint));

		if ((i % 3) == 0) {
			strcat(stringToPrint, "fizz");
		}

		if ((i % 5) == 0) {
			strcat(stringToPrint, "buzz");
		}

		if (strlen(stringToPrint) == 0) {
			/* might as well print directly */
			// sprintf(stringToPrint, "%d", i);
			printf("%d\n", i);
		}
		else {
			puts(stringToPrint);
		}
	}
}

/*
 * ---------------------------------------------------------------- Function ---
 * solves fizzbuzz style exercice for given range
 * branchless
 *
 * prints from look up table
 *   NOT(remainder) multiplied by relevant index
 *
 *     -> void
 *
 * time  O(n)
 * space O(1)
 */
void solve_fizzbuzz_branchless(const unsigned int step)
{
	const char *stringsToPrint[] = {"%d\n",
	                                "fizz\n",
	                                // stringsToPrint[3] + 4, //  possible ?
	                                "buzz\n",
	                                "fizzbuzz\n"};

	printf(stringsToPrint[!(step % 3) + !(step % 5) * 2], step);
}

void polyrhythm_fizzbuzz(const unsigned int step)
{
	const char *fizz[] = {"", "", "fizz"};
	const char *buzz[] = {"", "", "", "", "buzz"};
	printf("%u %s%s\n", step + 1, fizz[step % 3], buzz[step % 5]);
}

void polyrhythm_fizzbuzz_correct(const unsigned int step)
{
	const unsigned int is_fizz = step % 3;
	const unsigned int is_buzz = step % 5;

	const char *format[] = {"%s%s\n", "%.0s%.0s%u\n"};
	const char *fizz[]   = {"", "", "fizz"};
	const char *buzz[]   = {"", "", "", "", "buzz"};

	// printf("step : %u | is_fizz : %u | is_buzz : %u | format %u / %u\n",
	//        step + 1,
	//        is_fizz,
	//        is_buzz,
	//        !(buzz[is_buzz][0]) & !(fizz[is_fizz][0]),
	//        !((step + 1) % 3) + !((step + 1) % 5) * 2);

	printf(format[!(buzz[is_buzz][0]) & !(fizz[is_fizz][0])],
	       fizz[is_fizz],
	       buzz[is_buzz],
	       step + 1);
}

void polyrhythm_unwrapped_fizzbuzz(const unsigned int step)
{
	const char *fizzbuzz[] = {
	    "%u\n",
	    "%u\n",
	    "fizz\n",
	    "%u\n",
	    "buzz\n",
	    "fizz\n",
	    "%u\n",
	    "%u\n",
	    "fizz\n",
	    "buzz\n",
	    "%u\n",
	    "fizz\n",
	    "%u\n",
	    "%u\n",
	    "fizzbuzz\n",
	};
	printf(fizzbuzz[step % 15], step + 1);
}

/*
 * ---------------------------------------------------------------- Function ---
 * solves fizzbuzz style exercice for given range
 * branchless
 * silly
 * does not meet the requirements, the number is always printed
 *
 *     -> void
 *
 * time  O(n)
 * space O(1)
 */
void solve_fizzbuzz_silly(int range)
{
	/*  */
	const char stringsToPrint[] = {
	    'f', 'i', 'z', 'z', 'b', 'u', 'z', 'z', '\0', '%', 'd', '\0'};

	int index, length;

	for (int i = 1; i <= range; i++) {

		index  = !((i % 3) == 0) * !(i % 5) * 4;
		length = (!(i % 3) * 4 + !(i % 5) * 4);

		// printf("%d %d \n", index, length);
		/* see
		 * https://stackoverflow.com/questions/256218/the-simplest-way-of-printing-a-portion-of-a-char-in-c
		 */
		printf("%d %.*s\n", i, length, stringsToPrint + index);
	}
}

void switchbuzz(const unsigned int step)
{
	switch (step % 15) {
	case 3:
		printf("fizz\n");
		break;
	case 5:
		printf("buzz\n");
		break;
	case 6:
		printf("fizz\n");
		break;
	case 9:
		printf("fizz\n");
		break;
	case 10:
		printf("buzz\n");
		break;
	case 12:
		printf("fizz\n");
		break;
	case 15:
		printf("fizzbuzz\n");
		break;
	default:
		printf("%u\n", step);
		break;
	}
}

void switchbuzzthru(const unsigned int step)
{
	switch (step % 15) {
	case 3:
		printf("fizz\n");
		break;
	case 5:
		printf("buzz\n");
		break;
	case 6:
	case 9:
		printf("fizz\n");
		break;
	case 10:
		printf("buzz\n");
		break;
	case 12:
		printf("fizz\n");
		break;
	case 15:
		printf("fizzbuzz\n");
		break;
	default:
		printf("%u\n", step);
		break;
	}
}

void ratchet_fizzbuzz()
{
	const char *fizzbuzz[] = {
	    "%u\n",
	    "%u\n",
	    "fizz\n",
	    "%u\n",
	    "buzz\n",
	    "fizz\n",
	    "%u\n",
	    "%u\n",
	    "fizz\n",
	    "buzz\n",
	    "%u\n",
	    "fizz\n",
	    "%u\n",
	    "%u\n",
	    "fizzbuzz\n",
	};

	size_t ratchet = 0;
	size_t step    = 0;
	size_t i       = 0;
	do {
		printf(fizzbuzz[step], i + 1);
		if ((step = i - ratchet) == 14) {
			ratchet += 15;
		}
		i++;
	} while (i < LOOP_SIZE);
}
/*
 * -------------------------------------------------------------------- main ---
 * is the program entry point
 *
 *     -> error code
 *
 * time  O(n)
 * space O(1)
 */
int main()
{ // int argc, char const *argv[]) {

	// solve_fizzbuzz(100);
	// solve_fizzbuzz_branchless(10000);
	// solve_fizzbuzz_silly(100);

	int backup, new;
	fflush(stdout);
	backup = dup(1);
	new    = open("/dev/null", O_WRONLY);
	dup2(new, 1);
	close(new);

	float results[TEST_COUNT];

	BENCH(polyrhythm_fizzbuzz, LOOP_SIZE, results[0]);
	BENCH(polyrhythm_unwrapped_fizzbuzz, LOOP_SIZE, results[1]);
	BENCH(polyrhythm_fizzbuzz_correct, LOOP_SIZE, results[2]);
	BENCH(solve_fizzbuzz_branchless, LOOP_SIZE, results[3]);

	BENCH(solve_fizzbuzz_branchless, LOOP_SIZE, results[4]);
	BENCH(solve_fizzbuzz_branchless, LOOP_SIZE, results[5]);
	BENCH(solve_fizzbuzz_branchless, LOOP_SIZE, results[6]);

	BENCH(polyrhythm_unwrapped_fizzbuzz, LOOP_SIZE, results[7]);
	BENCH(polyrhythm_unwrapped_fizzbuzz, LOOP_SIZE, results[8]);
	BENCH(polyrhythm_unwrapped_fizzbuzz, LOOP_SIZE, results[9]);

	BENCH(switchbuzz, LOOP_SIZE, results[10]);
	BENCH(switchbuzz, LOOP_SIZE, results[11]);
	BENCH(switchbuzz, LOOP_SIZE, results[12]);

	BENCH(switchbuzzthru, LOOP_SIZE, results[13]);
	BENCH(switchbuzzthru, LOOP_SIZE, results[14]);
	BENCH(switchbuzzthru, LOOP_SIZE, results[15]);


	{
		float BENCH_start__, BENCH_end__, BENCH_diff__;
		START_BENCH(BENCH_start__);
		ratchet_fizzbuzz();
		STOP_BENCH(BENCH_start__, BENCH_end__, BENCH_diff__, results[16]);
	}
	// printf("test ratchet_fizzbuzz   \t: %f \n", results[16]);

	{
		float BENCH_start__, BENCH_end__, BENCH_diff__;
		START_BENCH(BENCH_start__);
		ratchet_fizzbuzz();
		STOP_BENCH(BENCH_start__, BENCH_end__, BENCH_diff__, results[17]);
	}
	// printf("test ratchet_fizzbuzz   \t: %f \n", results[17]);


	fflush(stdout);
	dup2(backup, 1);
	close(backup);

	for (size_t i = 0; i < TEST_COUNT; i++) {
		printf("test %lu   \t: %f \n", i, results[i]);
	}
	return 0;
}
