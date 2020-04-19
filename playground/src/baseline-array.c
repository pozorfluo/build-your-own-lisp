#include <stddef.h> /* size_t */
#include <stdint.h> /* uint32_t, uint64_t */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ansi_esc.h"

//---------------------------------------------------------------- BENCHMARK ---
#define BENCHMARK

#ifdef BENCHMARK
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#include <x86intrin.h> /* __rdtsc */

// #define LOOP_SIZE 10000000

#define START_BENCH(start)                                                     \
	do {                                                                       \
		start = (float)clock() / CLOCKS_PER_SEC;                               \
	} while (0)

#define STOP_BENCH(start, stop, diff, result)                                  \
	do {                                                                       \
		stop   = (float)clock() / CLOCKS_PER_SEC;                              \
		diff   = stop - start;                                                 \
		result = diff;                                                         \
		printf("|><| %f s\n", result);                                                 \
	} while (0)

#else
#define LOOP_SIZE 0
#define START_BENCH(start)
#define STOP_BENCH(start, stop, diff, result)
#define BENCH(expression, loop, result)
#endif /* BENCHMARK */

//------------------------------------------------------------- DECLARATIONS ---
struct hmap_entry {
	size_t key;
	size_t value;
};

//--------------------------------------------------------------------- MAIN ---
int main(void)
{
	srand(__rdtsc());

	size_t n = 8;
	float load_factor;

	fputs(FG_BRIGHT_BLUE REVERSE
	      "Table size is 2^n. Enter n ( default n = 8 ) ? " RESET,
	      stdout);
	scanf("%lu", &n);

	size_t capacity = (n < 1) ? (1u << 1) : (1u << n);
	struct hmap_entry *const hashmap =
	    malloc(sizeof(struct hmap_entry) * capacity);

	fputs(FG_BLUE REVERSE "Enter desired load factor ? " RESET, stdout);
	scanf("%f", &load_factor);
	//-------------------------------------------------------------- benchmark
	// setup
	// #ifdef BENCHMARK
	float start, stop, diff, bench_time;
// #endif /* BENCHMARK */

//-------------------------------------------------------------------- setup
#define TEST_COUNT 1000
	// size_t test_count = (1 << n); // 1 << (n - 1);
	size_t load_count = (1 << n) * load_factor;
	printf("load_factor = %f\n", load_factor);
	printf("load_count  = %lu\n", load_count);
	size_t sum_value = 0;
	char key[256];

	printf(FG_BRIGHT_YELLOW REVERSE "Filling hashmap with %lu entries\n" RESET,
	       load_count);

	START_BENCH(start);
	for (size_t k = 0; k < load_count; k++) {
		hashmap[k].key   = k;
		hashmap[k].value = k;
	}
	printf(FG_BRIGHT_YELLOW REVERSE "Done !\n" RESET);
	fgets(key, 255, stdin);
	//----------------------------------------------------------- input loop
	for (;;) {
		STOP_BENCH(start, stop, diff, bench_time);
		fputs("\x1b[102m > \x1b[0m", stdout);
		// scanf("%s", key);
		fgets(key, 255, stdin);
		size_t length = strlen(key);

		START_BENCH(start);
		/* trim newline */
		if ((length > 0) && (key[--length] == '\n')) {
			key[length] = '\0';
		}

		//-------------------------------------------------- exit
		if ((strcmp(key, "exit")) == 0) {
			break;
		}

		//-------------------------------------------------- rm
		if ((strcmp(key, "rm")) == 0) {
			for (size_t k = 0; k < load_count; k++) {
				hashmap[k].key   = 0;
				hashmap[k].value = 0;
			}
			continue;
		}

		//-------------------------------------------------- fill
		if ((strcmp(key, "fill")) == 0) {
			for (size_t k = 0; k < load_count; k++) {
				hashmap[k].key   = k;
				hashmap[k].value = k;
			}
			continue;
		}

		//-------------------------------------------------- find
		/**
		 * Test for values that do NOT exist with TEST_COUNT > load_count
		 */
		if ((strcmp(key, "find")) == 0) {
			sum_value = 0;
			for (size_t k = 0; k < TEST_COUNT; k++) {
				// printf("\tk = %lu\n", k);
				for (size_t i = 0; i < capacity; i++) {
					if (hashmap[i].key == k) {
						sum_value += hashmap[i].value;
						break;
					}
				}
			}
			printf("sum : %lu\n", sum_value);

			for (int k = TEST_COUNT - 1; k >= 0; k--) {
				// printf("\tk = %d\n", k);
				for (size_t i = 0; i < capacity; i++) {
					if (hashmap[i].key == (size_t)k) {
						sum_value -= hashmap[i].value;
						break;
					}
				}
			}
			printf("sum : %lu\n", sum_value);

			continue;
		}
		//-------------------------------------------------- findrand
		if ((strcmp(key, "findrand")) == 0) {
			sum_value = 0;
			for (size_t k = TEST_COUNT; k > 0; k--) {
				// printf("\tk = %lu\n", k);
				size_t random_key = rand() % capacity;
				// printf("random_key : %lu\n", random_key);
				for (size_t i = 0; i < capacity; i++) {
					if (hashmap[i].key == random_key) {
						// printf("found : %lu\n", hashmap[i].key);
						sum_value += hashmap[i].value;
						break;
					}
				}
			}
			printf("sum : %lu\n", sum_value);

			continue;
		}

		//-------------------------------------------------- dump
		if ((strcmp(key, "dump")) == 0) {
			for (size_t k = 0; k < load_count; k++) {
				printf("%lu | %lu\n", hashmap[k].key, hashmap[k].value);
			}
			continue;
		}

		//-------------------------------------------------- sum
		if ((strcmp(key, "sum")) == 0) {
			size_t sum_key   = 0;
			size_t sum_value = 0;
			for (size_t k = 0; k < load_count; k++) {
				sum_key += hashmap[k].key;
				sum_value += hashmap[k].value;
			}
			printf("sum : %lu | %lu\n", sum_key, sum_value);
			continue;
		}
	}

	//-------------------------------------------------------------- cleanup
	free(hashmap);
	return 0;
}