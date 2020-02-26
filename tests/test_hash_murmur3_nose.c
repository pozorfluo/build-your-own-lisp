//------------------------------------------------------------------ SUMMARY ---
/**
 * Test hash_murmur3_nose
 *
 *   Does NOT assess murmur3 fitness as hash function
 *   Does NOT benchmark hash_murmur3_nose performance
 *   Test hash_murmur3_nose against known expected results
 *   Provide a simplistic CLI
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash_murmur3_nose.h"

//------------------------------------------------------------------- MACROS ---
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof((array)[0]))

#define COMPARE_HASH(_buffer, _expected, _variant, _format, ...)               \
	{                                                                          \
		snprintf(_buffer, 33, _format, ##__VA_ARGS__);                         \
		printf(#_variant " : \t%s -> ", _buffer);                              \
		if (strcmp(_buffer, _expected._variant) != 0) {                        \
			fputs("failed !\n", stdout);                                       \
			failed++;                                                          \
		}                                                                      \
		else {                                                                 \
			fputs("ok !\n", stdout);                                           \
		}                                                                      \
	}
//----------------------------------------------------- FORWARD DECLARATIONS ---
typedef struct Result Result;
typedef struct Test Test;
//------------------------------------------------------------- DECLARATIONS ---
struct Result {
	char *x86_32;
	char *x86_128;
	char *x64_128;
};

struct Test {
	char *input;
	uint32_t seed;
	Result expected;
};
//----------------------------------------------------------------- Function ---
/**
 * Run self-test
 *   -> Failed tests count
 *
 * todo
 *   - [ ] populate tests list from a file
 */
int hash_murmur3_nose_test(void)
{
	Test tests[] = {
	    {"The quick brown fox jumps over the lazy dog.",
	     47,
	     {"346f27d0",
	      "0ecfdc8d586eda764e8330b24e5397a5",
	      "98953e34212723178c94e6e0380f4d9f"}},

	    {"The quick brown fox jumps over the lazy dog",
	     47,
	     {"c6a93fec",
	      "d251f7a605fc03d60d44643701610f99",
	      "14f21edd7638538c9a7163652ce03925"}},

	    {"The quick brown fox jumps over the lazy dog",
	     48,
	     {"11198af5",
	      "1bf54ee955398641796d4d33f82cdbaa",
	      "feac043c649ce03a38ef7308d20478b2"}},

	    {"The quick brown fox jumps over the lazy dog",
	     0,
	     {"2e4ff723",
	      "2f1583c3ecee2c675d7bf66ce5e91d2c",
	      "e34bbc7bbc071b6c7a433ca9c49a9347"}},

	    {"The quick brown fox jumps over the lazy dog",
	     1223,
	     {"09a717ba",
	      "7d485923d325727ccaadcb738425e1e4",
	      "a2b41aca17b9f42f4a06834858c198c7"}},

	    {"The",
	     1223,
	     {"0e454da1",
	      "a7666fba8d90bf198d90bf198d90bf19",
	      "be3b28c1450a48dfd5cc7cffa8cfd487"}},

	    {"the",
	     1223,
	     {"037bad62",
	      "2f3263676ac616c16ac616c16ac616c1",
	      "5763e6365cb6d4bc0ac7d01f4bb13854"}},
	};
	int failed = 0;

	uint32_t hash_x86_32;
	Hash128 hash_x86_128, hash_x64_128;

	size_t input_length;
	char buffer[33];

	printf("size of uint32_t : %lu bytes\n", sizeof(uint32_t));
	printf("size of Hash128  : %lu bytes\n\n", sizeof(Hash128));

	for (size_t i = 0; i < ARRAY_LENGTH(tests); i++) {
		printf("\"%s\" %u %s %s %s\n",
		       tests[i].input,
		       tests[i].seed,
		       tests[i].expected.x86_32,
		       tests[i].expected.x86_128,
		       tests[i].expected.x64_128);

		input_length = strlen(tests[i].input);
		hash_x86_32 =
		    murmurhash3_x86_32(tests[i].input, input_length, tests[i].seed);
		hash_x86_128 =
		    murmurhash3_x86_128(tests[i].input, input_length, tests[i].seed);
		hash_x64_128 =
		    murmurhash3_x64(tests[i].input, input_length, tests[i].seed);

		COMPARE_HASH(buffer, tests[i].expected, x86_32, "%08x", hash_x86_32);
		COMPARE_HASH(buffer,
		             tests[i].expected,
		             x86_128,
		             "%016lx%016lx",
		             hash_x86_128.hi,
		             hash_x86_128.lo);
		COMPARE_HASH(buffer,
		             tests[i].expected,
		             x64_128,
		             "%016lx%016lx",
		             hash_x64_128.hi,
		             hash_x64_128.lo);
		putchar('\n');
	}
	return failed;
}

//--------------------------------------------------------------------- MAIN ---
int main(int argc, char **argv)
{
	uint32_t seed = 47;
	uint32_t hash32;
	Hash128 hash;
	size_t input_length;
	int failedtests_count;
	char *seed_end;

	//-------------------------------------------------------------- CLI
	switch (argc) {
	// Run self-test
	case 0:
	case 1:
		failedtests_count = hash_murmur3_nose_test();
		if (failedtests_count == 0) {
			puts("All tests passed !");
		}
		else {
			printf("%d test(s) failed.\n", failedtests_count);
		}
		return failedtests_count > 0;
	// Use given seed
	case 3:
		errno = 0;
		// Allow seed written in bases other than 10
		// Prefix with 0x for base 16
		// Prefix 0 for base 8
		seed = strtoul(argv[2], &seed_end, 0);

		// Abort on errors
		if ((errno != 0) && (seed == 0)) {
			perror("Error : strtoul() could NOT process given seed ");
			return EXIT_FAILURE;
		}
		// Abort if seed is only junk or prefixed with junk
		// Ignore junk after valid seed though
		// Note : a	leading 0 means base 8, thus 09 will yield 0 as 9
		//        is considered junk in base 8
		if (seed_end == argv[2]) {
			printf("Error : could NOT process given seed : junk !\n");
			return EXIT_FAILURE;
		}

	// Fall-through
	// Hash given string
	case 2:
		input_length = strlen(argv[1]);

		printf("Input   : \"%s\"\n", argv[1]);
		printf("Seed    : %u\n", seed);

		hash32 = murmurhash3_x86_32(argv[1], input_length, seed);
		printf("murmurhash3_x86_32  : %08x\n", hash32);

		hash = murmurhash3_x86_128(argv[1], input_length, seed);
		printf("murmurhash3_x86_128 : %016lx %016lx\n", hash.hi, hash.lo);

		hash = murmurhash3_x64(argv[1], input_length, seed);
		printf("murmurhash3_x64     : %016lx %016lx\n", hash.hi, hash.lo);
		break;
	// Show help
	default:
		printf("Usage : %s \"string to hash\" [<seed>]\n", argv[0]);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}