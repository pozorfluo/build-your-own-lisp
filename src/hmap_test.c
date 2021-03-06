//------------------------------------------------------------------ SUMMARY ---
/**
 *
 */
#include <errno.h>    /* strtoumax */
#include <inttypes.h> /* strtoumax */
#include <limits.h>   /* UINT_MAX */
#include <stddef.h>   /* size_t */
#include <stdint.h>   /* uint32_t, uint64_t */
#include <stdio.h>
#include <stdlib.h> /* rand */
#include <string.h> /* memcpy, strlen, strnlen */

#include "ansi_esc.h"
#include "hfunc.h"
#include "hmap.h"

#define BENCHMARK
#include "benchmark.h"

//------------------------------------------------------------ CONFIGURATION ---
#define TEST_COUNT 1000000
// #define TEST_REGISTER

#define WELCOME_MESSAGE                                                        \
	FG_BRIGHT_BLUE REVERSE                                                     \
	    " hmap version 0.26.3 " RESET FG_BRIGHT_BLUE                           \
	    " type exit to quit\n" RESET FG_BRIGHT_RED REVERSE                     \
	    "   todo \n" RESET FG_BRIGHT_RED                                       \
	    "  - [ ] Investigate invalid read when growing a hmap initialized "    \
	    "with a size <= 16\n"                                                  \
	    "  - [ ] Deal with chain going over HMAP_PROBE_LENGTH at the end of "  \
	    "the map by triggering a resize, wrapping around, ...\n"               \
	    "  - [ ] Consider an irregular unrolled linked list for the store\n"   \
	    "  - [ ] Redo Packing\n"                                               \
	    "    + [ ] Use the 6 bytes of padding in buckets for extra hash\n"     \
	    "    + [ ] Consider inlining entry in bucket\n"                        \
	    " - [x] Consider there may be alignment problems with memory pool "    \
	    "split. \n"                                                            \
	    "    + [x] Look for the C idiomatic way to split memory and prevent "  \
	    "this. \n"                                                             \
	    "    + [x] Consider that it may not be a problem as "                  \
	    "structures are properly aligned/padded here. \n" FG_BRIGHT_YELLOW     \
	    "  - [ ] Look for a portable __builtin_clzll alternative in "          \
	    "hmap_new\n"                                                           \
	    "  - [ ] Consider simple typedefs to configure k, v types. \n"         \
	    "  - [ ] Explore pointer tagging to keep track of value type/mode\n"   \
	    "  - [ ] Bench against array\n"                                        \
	    "    + [ ] Find break even point for hmap vs array\n" RESET            \
	        FG_BRIGHT_GREEN                                                    \
	    "  - [ ] Consider unions for k and v of types up to size of pointer "  \
	    "+ size_t for data length\n"                                           \
	    "  - [ ] Consider parameterizing size of k and v\n"                    \
	    "  - [ ] Consider for wraparound that some version of abseil hashmap " \
	    "replicates a probe sized chunk of data from the beginning at the "    \
	    "end\n" RESET

//------------------------------------------------------------ MAGIC NUMBERS ---

//------------------------------------------------------------------- MACROS ---
#define STRINGIFY(_arg) #_arg
#define XSTRINFIGY(_arg) STRINGIFY(_arg)
//------------------------------------------------------------- DECLARATIONS ---

//----------------------------------------------------- FORWARD DECLARATIONS ---

//----------------------------------------------------------------- Function ---
/**
 * Pretty print bits for given n bytes at given data pointer
 *   Handle little endian only
 *   -> nothing
 */
void print_bits(const size_t n, void const *const data)
{
	unsigned char const *bit = (unsigned char *)data;
	unsigned char byte;

	// for (int i = n - 1; i >= 0; i--) {
	for (size_t i = 0; i < n; i++) {
		for (int j = 7; j >= 0; j--) {
			byte = (bit[i] >> j) & 1;
			printf("%u", byte);
		}
		// putchar('.');
	}

	putchar('\n');
}
//----------------------------------------------------------------- Function ---
/**
 * Dump given Hashmap content using a
 *   -> nothing
 */
void dump_hashmap_horizontal(const struct hmap *const hm,
                             size_t offset,
                             size_t limit,
                             size_t highlight,
                             size_t target)
{
	if (offset > hm->capacity) {
		offset = hm->capacity - HMAP_PROBE_LENGTH;
		limit  = HMAP_PROBE_LENGTH;
	};

	/* bucket numbers */
	for (size_t i = offset, j = 0; (i < hm->capacity) && (j < limit);
	     i++, j++) {
		printf(" %.2lu ", i);
	}
	putchar('\n');

	/* values */
	for (size_t i = offset, j = 0; (i < hm->capacity) && (j < limit);
	     i++, j++) {
		if (hm->buckets[i].meta == META_EMPTY) {
			// fputs("[  ]", stdout);
			printf("%s[  ]" RESET,
			       (i == highlight || i == target) ? REVERSE : "");
		}
		else {
			printf("%s[" FG_YELLOW "%.2lu" RESET "%s]" RESET,
			       (i == highlight || i == target) ? REVERSE : "",
			       hm->store[(hm->buckets[i].entry)].value,
			       (i == highlight || i == target) ? REVERSE : "");
		}
	}
	putchar('\n');

	/* distances */
	for (size_t i = offset, j = 0; (i < hm->capacity) && (j < limit);
	     i++, j++) {
		if (hm->buckets[i].meta == META_EMPTY) {
			fputs("[  ]", stdout);
		}
		else {

			printf("[" FG_RED "%.2d" RESET "]", hm->buckets[i].distance);
		}
	}
	putchar('\n');
}
//----------------------------------------------------------------- Function ---
/**
 * Dump given Hashmap content
 *   -> nothing
 */
void dump_hashmap(const struct hmap *const hm, size_t offset, size_t limit)
{
	if (offset > hm->capacity) {
		offset = hm->capacity - HMAP_PROBE_LENGTH;
		limit  = HMAP_PROBE_LENGTH;
	};

	// if ((offset + limit) > hm->capacity) {
	// 	limit = hm->capacity - offset;
	// }

	size_t empty_bucket = 0;
	int max_distance    = 0;
	// int last_at_home    = 0;
	size_t home_cursor  = 0;
	const char *key;

	for (size_t i = offset, j = 0; (i < hm->capacity) && (j < limit);
	     i++, j++) {
		if (hm->buckets[i].meta == META_EMPTY) {
			empty_bucket++;
			printf("\x1b[100m hmap->\x1b[30mbucket[%lu]>> EMPTY <<\x1b[0m\n",
			       i);
			// last_at_home = 0;
			continue;
		}

		// if (hm->buckets[i].distance == 0) {
		// 	last_at_home = 0;
		// }
		// else {
		// 	last_at_home++;
		// }
		// hm->buckets[i].distance == 0 ? last_at_home = 0 : last_at_home++;
		/* Color code distance from home using ANSI esc code values */
		int colour;
		key = hm->store[(hm->buckets[i].entry)].key;

		switch (hm->buckets[i].distance) {
		case 0:
			colour = 7; // WHITE
			break;
		case 1:
			colour = 2; // GREEN
			break;
		case 2:
			colour = 6; // CYAN
			break;
		case 3:
			colour = 4; // BLUE
			break;
		case 4:
			colour = 5; // MAGENTA
			break;
		case 5:
			colour = 3; // YELLOW
			break;
		default:
			colour = 1; // RED
			break;
		}
		max_distance = (hm->buckets[i].distance > max_distance)
		                   ? hm->buckets[i].distance
		                   : max_distance;
		// size_t home = (HREDUCE(HFUNC(key, strnlen(key,
		// HMAP_INLINE_KEY_SIZE)),
		//    hm->hash_shift));
		size_t home = (HREDUCE(HFUNC(key, strnlen(key, HMAP_INLINE_KEY_SIZE)),
		                       hm->hash_shift)) >>
		              15;

		printf("\x1b[9%dm" REVERSE " hm->bucket[%lu]>>" RESET, colour, i);

		// if ((home < home_cursor) || (hm->buckets[i].distance > last_at_home)
		// ||
		//     (hm->buckets[i].distance > (hm->buckets[i - 1].distance + 1))) {
		if (home < home_cursor) {
			printf(FG_BRIGHT_RED);
		}
		else {
			printf(FG_BRIGHT_BLACK);
			home_cursor = home;
		}
		printf(REVERSE " home[%lu] d[%d] m[%d] stored @[%u] " RESET,
		       home,
		       // hash_index(HREDUCE(HFUNC(key, strnlen(key,
		       // HMAP_INLINE_KEY_SIZE)),
		       //                    hm->hash_shift)),
		       hm->buckets[i].distance,
		       hm->buckets[i].meta,
		       hm->buckets[i].entry);

		// printf("\n%*.*s | ", HMAP_INLINE_KEY_SIZE, HMAP_INLINE_KEY_SIZE,
		// key);
		printf("     \t%*.*s | %lu \n",
		       HMAP_INLINE_KEY_SIZE,
		       HMAP_INLINE_KEY_SIZE,
		       key,
		       hm->store[(hm->buckets[i].entry)].value);

		// print_bits(HMAP_INLINE_KEY_SIZE, key);
	}
	// printf("empty_buckets       : %lu \t-> %f%%\n",
	//        empty_bucket,
	//        (double)empty_bucket / (double)(hm->capacity -
	//        HMAP_PROBE_LENGTH)
	//        *
	//            100);
	// printf("max_distance        : %d\n", max_distance);
}
//----------------------------------------------------------------- Function ---
/**
 * Dump given Hashmap store
 *   -> nothing
 */
void dump_store(const struct hmap *const hm)
{
	// for (size_t i = 0; i < hm->top; i++) {
	for (size_t i = 0; i < hm->store_capacity; i++) {
		if (i < hm->top) {
			printf(FG_BRIGHT_CYAN REVERSE "\n store @[%lu] " RESET, i);
		}
		else {
			printf(FG_BRIGHT_BLACK REVERSE "\n store @[%lu] " RESET, i);
		}
		printf("%*.*s | %lu\n",
		       HMAP_INLINE_KEY_SIZE,
		       HMAP_INLINE_KEY_SIZE,
		       hm->store[i].key,
		       hm->store[i].value);
		print_bits(HMAP_INLINE_KEY_SIZE, hm->store[i].key);
	}
}

//----------------------------------------------------------------- Function ---
/**
 * Dump given Hashmap stats
 *   -> nothing
 */
void dump_stats(const struct hmap *const hm)
{
	size_t empty_bucket    = 0;
	size_t empty_chain     = 0;
	size_t max_empty_chain = 0;
	int max_distance       = 0;
	// const char *key;

	for (size_t i = 0; i < hm->capacity; i++) {
		if (hm->buckets[i].meta == META_EMPTY) {
			empty_bucket++;
			/* disregard empty chains past advertised capacity */
			if (i < (hm->capacity - HMAP_PROBE_LENGTH)) {
				empty_chain++;
			}
			continue;
		}

		// key = hm->store[(hm->buckets[i].entry)].key;
		max_empty_chain =
		    (empty_chain > max_empty_chain) ? empty_chain : max_empty_chain;
		empty_chain  = 0;
		max_distance = (hm->buckets[i].distance > max_distance)
		                   ? hm->buckets[i].distance
		                   : max_distance;
	}

	printf(FG_BRIGHT_YELLOW REVERSE "hmap->capacity       : %lu \n" RESET,
	       hm->capacity);
	printf(FG_YELLOW REVERSE "hmap->store_capacity : %lu \n" RESET,
	       hm->store_capacity);
	printf(FG_YELLOW REVERSE "hmap->top            : %lu \n" RESET, hm->top);

	printf("empty_buckets        : %lu -> %f%% \n",
	       empty_bucket,
	       (double)empty_bucket / (double)(hm->capacity - HMAP_PROBE_LENGTH) *
	           100);
	printf("max_distance         : %d \n", max_distance);
	printf("max_empty_chain      : %lu \n", max_empty_chain);
}

//----------------------------------------------------------------- Function ---
/**
 * Go through all buckets
 * Access the store
 *   -> nothing
 */
void sum_bucket(const struct hmap *const hm)
{
	size_t sum_key   = 0;
	size_t sum_value = 0;

	for (size_t i = 0; i < hm->capacity; i++) {
		// sum_key += hm->store[(hm->buckets[i].entry)].key;
		sum_value += hm->store[(hm->buckets[i].entry)].value;
	}

	printf(FG_BLUE REVERSE
	       "sum_key             : %lu\n"
	       "sum_value           : %lu\n" RESET,
	       sum_key,
	       sum_value);
}
//----------------------------------------------------------------- Function ---
size_t sum_store(const struct hmap *const hashmap)
{
	// size_t sum_key   = 0;
	size_t sum_value = 0;

	/* first rewind 1 entry from the top */
	size_t top = hashmap->top;
	while (top) {
		top--;
		// sum_key += hashmap->store[top].key;
		sum_value += hashmap->store[top].value;
	}
	return sum_value;
}
//----------------------------------------------------------------- Function ---
/**
 * Return a pseudo-random uint64_t number.
 */
uint64_t mcg64()
{
	static uint64_t seed = 1;
	return (seed = (164603309694725029ull * seed) % 14738995463583502973ull);
}
//--------------------------------------------------------------------- MAIN ---
/**
 *
 */
int main(void)
{
	puts(WELCOME_MESSAGE);
#ifdef __AVX__
	puts("__AVX__ 1");
#endif /* __AVX__ */

	printf("__WORDSIZE           %d\n", __WORDSIZE);
	printf("RAND_MAX             %d\n", RAND_MAX);
	printf("SIZE_MAX             %lu\n", SIZE_MAX);
	printf("UINT_MAX             %u\n", UINT_MAX);
	printf("UINT64_MAX           %lu\n", UINT64_MAX);
	printf("HMAP_NOT_FOUND       %lu\n", HMAP_NOT_FOUND);
	printf("size_t               %lu bytes\n", sizeof(size_t));
	printf("struct hmap          %lu bytes\n", sizeof(struct hmap));
	printf("struct hmap_bucket   %lu bytes\n", sizeof(struct hmap_bucket));
	printf("struct hmap_entry    %lu bytes\n", sizeof(struct hmap_entry));
	printf("struct meta_byte     %lu bytes\n", sizeof(meta_byte));
	puts("HFUNC                " XSTRINFIGY(HFUNC));
	puts("HREDUCE              " XSTRINFIGY(HREDUCE));
	puts("HCMP                 " XSTRINFIGY(HCMP));
	puts("HWIDTH               " XSTRINFIGY(HWIDTH));
	puts("HMAP_INLINE_KEY_SIZE " XSTRINFIGY(HMAP_INLINE_KEY_SIZE));
	puts("HMAP_MAX_LOAD        " XSTRINFIGY(HMAP_MAX_LOAD));
	puts("HMAP_PROBE_LENGTH    " XSTRINFIGY(HMAP_PROBE_LENGTH));
#ifdef HWIDTH_64
	puts("64");
#endif
	// puts("HCOPY   "XSTRINFIGY(HCOPY));

	// uint32_t seed = 31;
	size_t requested_capacity = 100;
	// float load_factor;
	int unused_result __attribute__((unused));
	char *unused_result_s __attribute__((unused));

	fputs(FG_BRIGHT_BLUE REVERSE
	      //   "Table size is 2^n. Enter n ( default n = 8 ) ? " RESET,
	      "Enter requested capacity ? \n" RESET,
	      stdout);
	fputs(FG_GREEN BG_BRIGHT_BLACK " > " RESET, stdout);
	unused_result = scanf("%lu", &requested_capacity);

	// struct hmap *const hashmap = hmap_new(requested_capacity);
	struct hmap *hashmap = hmap_new(requested_capacity);

	// fputs(FG_BLUE REVERSE "Enter desired load factor ? " RESET, stdout);
	// unused_result = scanf("%f", &load_factor);

	//---------------------------------------------------------------- setup
	SETUP_BENCH(repl);
	// size_t load_count = requested_capacity * load_factor;
	// printf("load_factor = %f\n", load_factor);
	// printf("load_count  = %lu\n", load_count);
	// size_t capacity  = hashmap->capacity;
	size_t sum_value = 0;
	char key[256];
	char random_key[HMAP_INLINE_KEY_SIZE + 1] = {'\0'};

	// printf(FG_BRIGHT_YELLOW REVERSE "Filling hashmap with %lu entries\n"
	// RESET,
	//        load_count);

	printf(FG_BRIGHT_YELLOW REVERSE "hmap->top : %lu\n" RESET, hashmap->top);

	START_BENCH(repl);

	// while (hashmap->top < load_count) {
	// 	*(uint64_t *)(random_key)                            = mcg64();
	// 	*(uint64_t *)(random_key + HMAP_INLINE_KEY_SIZE / 2) = mcg64();
	// 	hmap_put(hashmap, random_key, hashmap->top);
	// }

	size_t rand_length;
	// while (hashmap->top < load_count) {
	// 	rand_length = rand() % 15 + 1;
	// 	for (size_t i = 0; i < rand_length; i++) {
	// 		random_key[i] = (char)(rand() % 26 + 0x61);
	// 	}
	// 	random_key[rand_length + 1] = '\0';
	// 	hmap_put(hashmap, random_key, rand_length);
	// }

	printf(FG_BRIGHT_YELLOW REVERSE "Done !\n" RESET);
	printf(FG_YELLOW REVERSE "hmap->top : %lu\n" RESET, hashmap->top);
	unused_result_s = fgets(key, 255, stdin);
	//----------------------------------------------------------- input loop
	for (;;) {
		STOP_BENCH(repl);
		fputs(FG_GREEN BG_BRIGHT_BLACK " > " RESET, stdout);
		unused_result_s = fgets(key, 255, stdin);
		size_t length   = strlen(key);

		START_BENCH(repl);
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
			// for (size_t k = 0; k < load_count; k++) {
			size_t is_stuck = hashmap->top;
			while (hashmap->top) {
				// printf(FG_BRIGHT_YELLOW REVERSE
				//        "hmap->top : %lu"
				//        " : store[%lu]"
				//        " : k %.*s"
				//        " : v %lu"
				//        " : bucket[%lu]"
				//        " : strlen %lu\n" RESET,
				//        hashmap->top,
				//        hashmap->top - 1,
				//        HMAP_INLINE_KEY_SIZE,
				//        hashmap->store[hashmap->top - 1].key,
				//        hashmap->store[hashmap->top - 1].value,
				//        hmap_find(hashmap, hashmap->store[hashmap->top -
				//        1].key), strnlen(hashmap->store[hashmap->top - 1].key,
				//                HMAP_INLINE_KEY_SIZE));

				hmap_remove(hashmap, hashmap->store[hashmap->top - 1].key);
				if (hashmap->top == is_stuck) {
					break;
				}
				is_stuck = hashmap->top;
			}
			printf(FG_BRIGHT_YELLOW REVERSE "hmap->top : %lu\n" RESET,
			       hashmap->top);
			continue;
		}
		//-------------------------------------------------- fill
		if ((strcmp(key, "fill")) == 0) {
			const size_t fill_up_to = hashmap->store_capacity;
			while (hashmap->top < fill_up_to) {
				// *(uint64_t *)(random_key) = rand();
				// *(uint64_t *)(random_key + HMAP_INLINE_KEY_SIZE / 2) =
				//     hashmap->top;
				rand_length = rand() % 15 + 1;
				for (size_t i = 0; i < rand_length; i++) {
					random_key[i] = (char)(rand() % 26 + 0x61);
				}
				random_key[rand_length + 1] = '\0';
				hmap_put(hashmap, random_key, hashmap->top);
			}
			printf(FG_BRIGHT_YELLOW REVERSE "hmap->top : %lu\n" RESET,
			       hashmap->top);
			continue;
		}
		//-------------------------------------------------- fill10
		if ((strcmp(key, "fill10")) == 0) {
			size_t count = 10;
			while (count--) {
				// *(uint64_t *)(random_key) = rand();
				// *(uint64_t *)(random_key + HMAP_INLINE_KEY_SIZE / 2) =
				//     hashmap->top;
				rand_length = rand() % 15 + 1;
				for (size_t i = 0; i < rand_length; i++) {
					random_key[i] = (char)(rand() % 26 + 0x61);
				}
				random_key[rand_length + 1] = '\0';
				hmap_put(hashmap, random_key, hashmap->top);
			}
			printf(FG_BRIGHT_YELLOW REVERSE "hmap->top : %lu\n" RESET,
			       hashmap->top);
			continue;
		}
		//-------------------------------------------------- fill100
		if ((strcmp(key, "fill100")) == 0) {
			size_t count = 100;
			while (count--) {
				// *(uint64_t *)(random_key) = rand();
				// *(uint64_t *)(random_key + HMAP_INLINE_KEY_SIZE / 2) =
				//     hashmap->top;
				rand_length = rand() % 15 + 1;
				for (size_t i = 0; i < rand_length; i++) {
					random_key[i] = (char)(rand() % 26 + 0x61);
				}
				random_key[rand_length + 1] = '\0';
				hmap_put(hashmap, random_key, hashmap->top);
			}
			printf(FG_BRIGHT_YELLOW REVERSE "hmap->top : %lu\n" RESET,
			       hashmap->top);
			continue;
		}
		//-------------------------------------------------- fill1K
		if ((strcmp(key, "fill1K")) == 0) {
			size_t count        = 1000;
			size_t previous_key = 33;
			size_t key;
			while (count--) {
				key = ((HFIBO * rand()) & 0x7E7E7E7E7E7E7E7Ellu) |
				      0x2121212121212121llu;
				*(uint64_t *)(random_key)     = key;
				*(uint64_t *)(random_key + 8) = previous_key;
				previous_key                  = key;
				// *(uint64_t *)(random_key + HMAP_INLINE_KEY_SIZE / 2) =
				//     hashmap->top;
				// rand_length = rand() % 15 + 1;
				// for (size_t i = 0; i < rand_length; i++) {
				// 	random_key[i] = (char)(rand() % 26 + 0x61);
				// }
				// random_key[rand_length + 1] = '\0';
				hmap_put(hashmap, random_key, hashmap->top);
			}
			printf(FG_BRIGHT_YELLOW REVERSE "hmap->top : %lu\n" RESET,
			       hashmap->top);
			continue;
		}
		//-------------------------------------------------- fill1K
		if ((strcmp(key, "fill10K")) == 0) {
			size_t count        = 10000;
			size_t previous_key = 33;
			size_t key;
			while (count--) {
				key = ((HFIBO * rand()) & 0x7E7E7E7E7E7E7E7Ellu) |
				      0x2121212121212121llu;
				*(uint64_t *)(random_key)     = key;
				*(uint64_t *)(random_key + 8) = previous_key;
				previous_key                  = key;
				// *(uint64_t *)(random_key + HMAP_INLINE_KEY_SIZE / 2) =
				//     hashmap->top;
				// rand_length = rand() % 15 + 1;
				// for (size_t i = 0; i < rand_length; i++) {
				// 	random_key[i] = (char)(rand() % 26 + 0x61);
				// }
				// random_key[rand_length + 1] = '\0';
				hmap_put(hashmap, random_key, hashmap->top);
			}
			printf(FG_BRIGHT_YELLOW REVERSE "hmap->top : %lu\n" RESET,
			       hashmap->top);
			continue;
		}
		//-------------------------------------------------- fill10Ks
		if ((strcmp(key, "fill10Ks")) == 0) {
			size_t count = 10000;
			while (count--) {

				rand_length = rand() % 15 + 1;
				for (size_t i = 0; i < rand_length; i++) {
					random_key[i] = (char)(rand() % 26 + 0x61);
				}
				random_key[rand_length + 1] = '\0';
				hmap_put(hashmap, random_key, hashmap->top);
			}
			printf(FG_BRIGHT_YELLOW REVERSE "hmap->top : %lu\n" RESET,
			       hashmap->top);
			continue;
		}
		//-------------------------------------------------- fill1M
		if ((strcmp(key, "fill1M")) == 0) {
			size_t count        = 1000000;
			size_t previous_key = 33;
			size_t key;
			while (count--) {
				key = ((HFIBO * rand()) & 0x7E7E7E7E7E7E7E7Ellu) |
				      0x2121212121212121llu;
				*(uint64_t *)(random_key)     = key;
				*(uint64_t *)(random_key + 8) = previous_key;
				previous_key                  = key;
				// *(uint64_t *)(random_key + HMAP_INLINE_KEY_SIZE / 2) =
				//     hashmap->top;
				// rand_length = rand() % 15 + 1;
				// for (size_t i = 0; i < rand_length; i++) {
				// 	random_key[i] = (char)(rand() % 26 + 0x61);
				// }
				// random_key[rand_length + 1] = '\0';
				hmap_put(hashmap, random_key, hashmap->top);
			}
			printf(FG_BRIGHT_YELLOW REVERSE "hmap->top : %lu\n" RESET,
			       hashmap->top);
			continue;
		}
		//-------------------------------------------------- fill3M
		if ((strcmp(key, "fill3M")) == 0) {
			size_t count        = 3000000;
			size_t previous_key = 33;
			size_t key;
			while (count--) {
				key = ((HFIBO * rand()) & 0x7E7E7E7E7E7E7E7Ellu) |
				      0x2121212121212121llu;
				*(uint64_t *)(random_key)     = key;
				*(uint64_t *)(random_key + 8) = previous_key;
				previous_key                  = key;
				;
				hmap_put(hashmap, random_key, hashmap->top);
			}
			printf(FG_BRIGHT_YELLOW REVERSE "hmap->top : %lu\n" RESET,
			       hashmap->top);
			continue;
		}
		//-------------------------------------------------- fill3M
		if ((strcmp(key, "fill3Ms")) == 0) {
			size_t count = 3000000;
			while (count--) {

				rand_length = rand() % 15 + 1;
				for (size_t i = 0; i < rand_length; i++) {
					random_key[i] = (char)(rand() % 26 + 0x61);
				}
				random_key[rand_length + 1] = '\0';
				hmap_put(hashmap, random_key, hashmap->top);
			}
			printf(FG_BRIGHT_YELLOW REVERSE "hmap->top : %lu\n" RESET,
			       hashmap->top);
			continue;
		}
		//-------------------------------------------------- fill3M
		if ((strcmp(key, "fill3Mf")) == 0) {
			size_t count = 3000000;
			while (count--) {
				*(uint64_t *)(random_key) = rand();
				hmap_put(hashmap, random_key, hashmap->top);
			}
			printf(FG_BRIGHT_YELLOW REVERSE "hmap->top : %lu\n" RESET,
			       hashmap->top);
			continue;
		}
		//-------------------------------------------------- dump
		if ((strcmp(key, "dump")) == 0) {
			dump_hashmap(hashmap, 0, SIZE_MAX);
			continue;
		}
		//-------------------------------------------------- stats
		if ((strcmp(key, "stats")) == 0) {
			dump_stats(hashmap);
			continue;
		}
		//-------------------------------------------------- clear
		if ((strcmp(key, "clear")) == 0) {
			hmap_clear(hashmap);
			continue;
		}
		//-------------------------------------------------- sumb
		if ((strcmp(key, "sumb")) == 0) {
			sum_bucket(hashmap);
			continue;
		}

		//-------------------------------------------------- sum
		if ((strcmp(key, "sum")) == 0) {
			printf(FG_BLUE REVERSE "store sum_value        : %lu\n" RESET,
			       sum_store(hashmap));
			continue;
		}
		//-------------------------------------------------- store
		if ((strcmp(key, "store")) == 0) {
			dump_store(hashmap);
			continue;
		}
		//-------------------------------------------------- store
		if ((strcmp(key, "grow")) == 0) {
			debug_grow(hashmap);
			// load_count = hashmap->store_capacity;
			continue;
		}
		//-------------------------------------------------- rehash
		if ((strcmp(key, "rehash")) == 0) {
			debug_rehash(hashmap);
			// if (hashmap->top > 0) {
			// 	struct hmap *const hashmap_2x = hmap_new(load_count * 2);
			// 	printf(FG_BRIGHT_YELLOW REVERSE "hmap->top : %lu\n" RESET,
			// 	       hashmap->top);
			// 	size_t store_index = hashmap->top;
			// 	while (store_index--) {
			// 		hmap_put(hashmap_2x,
			// 		         hashmap->store[store_index].key,
			// 		         hashmap->store[store_index].value);
			// 		// result = HREDUCE(HFUNC(k, strnlen(k,
			// 		// HMAP_INLINE_KEY_SIZE)),
			// 		//                  hashmap->hash_shift);
			// 	}
			// 	// dump_stats(hashmap_2x);
			// 	// puts(FG_BRIGHT_MAGENTA REVERSE
			// 	//      "This is worse than reallocing the store and
			// rehashing
			// to"
			// 	//      " a bigger map." RESET);

			// 	hmap_free(hashmap);
			// 	hashmap = hashmap_2x;
			// 	load_count *= 2;
			// }
			continue;
		}

		//-------------------------------------------------- findall
		if ((strcmp(key, "findall")) == 0) {
			if (hashmap->top > 0) {
				printf(FG_BRIGHT_YELLOW REVERSE "hmap->top : %lu\n" RESET,
				       hashmap->top);
				size_t store_index = hashmap->top;
				size_t result, found_store_index;
				while (store_index--) {
					result =
					    hmap_find(hashmap, hashmap->store[store_index].key);
					found_store_index = hashmap->buckets[result].entry;
					if (result == HMAP_NOT_FOUND) {
						puts("Key not found ! \n");
					}

					if (found_store_index != store_index) {
						printf(FG_BRIGHT_RED REVERSE " [ Find error !] " RESET);
					}
					else {
						printf(FG_BRIGHT_GREEN REVERSE " [ OK ] " RESET);
					}
					printf(FG_BRIGHT_YELLOW REVERSE
					       " store[%lu]:found[%lu]:bucket[%lu] " RESET
					       "    \t : %*.*s"
					       " | %.lu \n",
					       store_index,
					       found_store_index,
					       result,
					       HMAP_INLINE_KEY_SIZE,
					       HMAP_INLINE_KEY_SIZE,
					       hashmap->store[store_index].key,
					       hashmap->store[store_index].value);
				}
			}
			else {
				printf(FG_BRIGHT_RED REVERSE
				       "hmap->top : %lu | hashmap is empty\n" RESET,
				       hashmap->top);
			}
			continue;
		}
		//-------------------------------------------------- findallsilent
		if ((strcmp(key, "findallsilent")) == 0) {
			if (hashmap->top > 0) {
				printf(FG_BRIGHT_YELLOW REVERSE "hmap->top : %lu\n" RESET,
				       hashmap->top);
				size_t store_index = hashmap->top;
				size_t result, found_store_index;
				size_t not_found_count = 0;
				size_t error_count     = 0;
				sum_value              = 0;

				while (store_index--) {
					result =
					    hmap_find(hashmap, hashmap->store[store_index].key);
					if (result == HMAP_NOT_FOUND) {
						not_found_count++;
					}
					else {
						found_store_index = hashmap->buckets[result].entry;
						if (found_store_index != store_index) {
							error_count++;
						}
						sum_value += hashmap->store[found_store_index].value;
					}
				}
				if (error_count | not_found_count) {
					printf(FG_BRIGHT_RED REVERSE
					       " [ NOK ] " RESET FG_BRIGHT_RED BG_BLACK
					       " %lu error(s), %lu existing key(s) but not "
					       "found.\n",
					       error_count,
					       not_found_count);
				}
				else {
					printf(FG_BRIGHT_GREEN REVERSE " [ OK ] \n" RESET);
				}

				printf(FG_BLUE REVERSE
				       "store sum_value        : %lu\n" RESET FG_BRIGHT_BLUE
				           REVERSE "found sum_value        : %lu\n" RESET,
				       sum_store(hashmap),
				       sum_value);
			}
			else {
				printf(FG_BRIGHT_RED REVERSE
				       "hmap->top : %lu | hashmap is empty\n" RESET,
				       hashmap->top);
			}
			continue;
		}
		//-------------------------------------------------- findin
		if ((strcmp(key, "findin")) == 0) {
			sum_value = 0;

			for (size_t k = 0; k < TEST_COUNT; k++) {
				sum_value +=
				    hmap_get(hashmap, hashmap->store[k % hashmap->top].key);
			}

			printf("sum : %lu\nTEST_COUNT : %d\n", sum_value, TEST_COUNT);
			continue;
		}
		//-------------------------------------------------- findrandin
		if ((strcmp(key, "findrandin")) == 0) {
			sum_value = 0;

			for (size_t k = 0; k < TEST_COUNT; k++) {
				sum_value += hmap_get(
				    hashmap, hashmap->store[rand() % hashmap->top].key);
			}

			printf("sum : %lu\nTEST_COUNT : %d\n", sum_value, TEST_COUNT);
			continue;
		}
		//-------------------------------------------------- findrand
		if ((strcmp(key, "findrand")) == 0) {
			sum_value = 0;
			for (size_t k = 0; k < TEST_COUNT; k++) {
				*(uint64_t *)(random_key)                            = mcg64();
				*(uint64_t *)(random_key + HMAP_INLINE_KEY_SIZE / 2) = mcg64();
				sum_value += hmap_get(hashmap, random_key);
			}
			printf("sum : %lu\nTEST_COUNT : %d\n", sum_value, TEST_COUNT);
			continue;
		}

		if ((strcmp(key, "add")) == 0) {
			printf("todo\n");
			continue;
		}

		//------------------------------------------------------- find
		// bucket
		if (key[0] == '@') {
			printf("%s\n", key + 1);

			char *key_end;
			size_t bucket;
			errno = 0;
			// Allow bucket number written in bases other than 10
			// Prefix with 0x for base 16
			// Prefix 0 for base 8
			bucket = strtoumax(key + 1, &key_end, 0);

			// Abort on errors
			if ((errno != 0) && (bucket == 0)) {
				perror(
				    "Error : strtoumax () could NOT process given bucket "
				    "number ");
				continue;
			}
			// Abort if bucket number is only junk or prefixed with junk
			// Ignore junk after valid bucket number though
			// Note : a leading 0 means base 8, thus 09 will yield 0 as 9
			//        is considered junk in base 8
			if (key_end == key) {
				printf(
				    "Error : could NOT process given bucket number : junk !\n");
				continue;
			}
			size_t entry     = hashmap->buckets[bucket].entry;
			char *bucket_key = hashmap->store[entry].key;

			size_t result = hmap_find(hashmap, bucket_key);

			printf(FG_BRIGHT_BLACK REVERSE "\n store @[%lu] " RESET, entry);
			printf("%*.*s | %lu\n",
			       HMAP_INLINE_KEY_SIZE,
			       HMAP_INLINE_KEY_SIZE,
			       hashmap->store[entry].key,
			       hashmap->store[entry].value);
			print_bits(HMAP_INLINE_KEY_SIZE, hashmap->store[entry].key);

			dump_hashmap(hashmap, bucket, 30);

			printf("hashes to : %lu\n",
			       (HREDUCE(HFUNC(bucket_key,
			                      strnlen(bucket_key, HMAP_INLINE_KEY_SIZE)),
			                hashmap->hash_shift)));
			// printf("hashes to : %lu\n",
			//        (HREDUCE(HFUNC(bucket_key,
			//                       strnlen(bucket_key, HMAP_INLINE_KEY_SIZE)),
			//                 hashmap->hash_shift)) >>
			//            15);

			if (result == HMAP_NOT_FOUND) {
				puts("Key not found ! \n");
			}
			else {
				printf(
				    "Looking for %.16s -> found @ %lu\n", bucket_key, result);
				printf("\t\t-> value : %lu\n", hmap_get(hashmap, bucket_key));
			}
			// dump_hashmap(hashmap, bucket - 10, 20);
			continue;
		}
		//------------------------------------------------------- find / put
		if (key[0] != '\0') {
			size_t result = hmap_find(hashmap, key);
			// printf("hashes to : %lu\n",
			//        (HREDUCE(HFUNC(key, strnlen(key, HMAP_INLINE_KEY_SIZE)),
			//                 hashmap->hash_shift)));
			printf("hashes to : %lu\n",
			       (HREDUCE(HFUNC(key, strnlen(key, HMAP_INLINE_KEY_SIZE)),
			                hashmap->hash_shift)) >>
			           15);

			if (result == HMAP_NOT_FOUND) {
				puts("Key not found ! \n");
			}
			else {
				printf("Looking for %.16s -> found @ %lu\n", key, result);
				printf("\t\t-> value : %lu\n", hmap_get(hashmap, key));
				printf("Removing entry !\n");
				hmap_remove(hashmap, key);
				printf(FG_YELLOW REVERSE "hmap->top : %lu\n" RESET,
				       hashmap->top);
			}
			continue;
		}
	}

	//-------------------------------------------------------------- cleanup
	hmap_free(hashmap);
	return 0;
}