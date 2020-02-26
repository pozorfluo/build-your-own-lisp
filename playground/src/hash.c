//------------------------------------------------------------------ SUMMARY ---
/**
 * Describe the plan for a hash table as I currently understand it :
 *   Generate hashes from a large "hash space" for given key set
 *   Use an "array" big enough relative to key set and desired load factor
 *     but smaller than "hash space" as hash map
 *   Use hash % (map size) as index to ...
 *   Insert/Append KeyValue pairs along with hash in a linked list
 *   Resize map when/if load factor threshold is crossed
 *     this should NOT require to recompute all the hashes
 *   Handle map index collision with search in linked list
 *   Figure out if/how to handle actual hash collision
 *
 * todo
 *   - [ ] Implement basic multiplicative hash function
 *   - [ ] Port murmur3 to C to suit your needs
 *   - [ ] Include xxhash
 *   - [ ] Compare and select hash function
 *   - [ ] Consider secondary hash map for collisions
 *     + [ ] See :
 * http://courses.cs.vt.edu/~cs3114/Fall09/wmcquain/Notes/T17.PerfectHashFunctions.pdf
 *   - [ ] Consider quadratic probing for collisions
 * 	   + [ ] See :
 * https://stackoverflow.com/questions/22437416/best-way-to-resize-a-hash-table
 * 	   + [ ] See : https://github.com/jamesroutley/write-a-hash-table
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/hash_murmur3_nose.h"
//------------------------------------------------------------- DEBUG MACROS ---
/**
 * Helps me follow malloc / free where needed
 */
#ifdef DEBUG_MALLOC

#include <malloc.h>

void *xmalloc(size_t size, const char *origin, const char *destination)
{
	void *return_pointer = malloc(size);
	if ((return_pointer == NULL) && !size) {
		return_pointer = malloc(1);
	}
	if ((return_pointer == NULL)) {
		printf(FG_RED REVERSE
		       "malloc failed, Out of memory I guess\n"
		       "\tcalled inside      : %s()\n"
		       "\ttried to allocate  : %lu bytes\n"
		       "\tfor                : %s\n" RESET,
		       origin,
		       size,
		       destination);
		exit(EXIT_FAILURE);
	}

	printf(FG_CYAN "\tmallocing %s %lu bytes inside %s()\n" RESET,
	       destination,
	       size,
	       origin);
	memset(return_pointer, 0xFB, size);
	return return_pointer;
}

void xfree(void *pointer, const char *pointer_name, const char *origin)
{
	size_t bytes_in_malloced_block = malloc_usable_size(pointer);
	printf(FG_BLUE "\tfreeing %s %lu bytes inside %s()\n" RESET,
	       pointer_name,
	       bytes_in_malloced_block,
	       origin);
	free(pointer);
}

#define XMALLOC(_size, _origin, _destination)                                  \
	xmalloc(_size, _origin, _destination)

#define XFREE(_pointer, _origin) xfree(_pointer, #_pointer, _origin)

#else
#define XMALLOC(_size, _origin, _destination) malloc(_size)
#define XFREE(_pointer, _origin) free(_pointer)
#endif

//------------------------------------------------------------------- MACROS ---
#define ARRAY_LENGTH(array) (sizeof(array) / sizeof((array)[0]))

//------------------------------------------------------------- DECLARATIONS ---
typedef struct KeyValuePair {
	// todo
	// - [ ] Study KeyValuePair data alignment
	size_t hash;
	char *key;
	void *value;
} KeyValuePair;

//----------------------------------------------------------- FILE SCOPE LUT ---
// first 200 prime numbers
static const uint_fast16_t prime_numbers[200] = {
    2,    3,    5,    7,    11,   13,   17,   19,   23,   29,   31,   37,
    41,   43,   47,   53,   59,   61,   67,   71,   73,   79,   83,   89,
    97,   101,  103,  107,  109,  113,  127,  131,  137,  139,  149,  151,
    157,  163,  167,  173,  179,  181,  191,  193,  197,  199,  211,  223,
    227,  229,  233,  239,  241,  251,  257,  263,  269,  271,  277,  281,
    283,  293,  307,  311,  313,  317,  331,  337,  347,  349,  353,  359,
    367,  373,  379,  383,  389,  397,  401,  409,  419,  421,  431,  433,
    439,  443,  449,  457,  461,  463,  467,  479,  487,  491,  499,  503,
    509,  521,  523,  541,  547,  557,  563,  569,  571,  577,  587,  593,
    599,  601,  607,  613,  617,  619,  631,  641,  643,  647,  653,  659,
    661,  673,  677,  683,  691,  701,  709,  719,  727,  733,  739,  743,
    751,  757,  761,  769,  773,  787,  797,  809,  811,  821,  823,  827,
    829,  839,  853,  857,  859,  863,  877,  881,  883,  887,  907,  911,
    919,  929,  937,  941,  947,  953,  967,  971,  977,  983,  991,  997,
    1009, 1013, 1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069,
    1087, 1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163,
    1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223};

// as seen in cpp stl
static const size_t prime_sizes[62] = {
    /* 0  */ 5ul,
    /* 1  */ 11ul,
    /* 2  */ 23ul,
    /* 3  */ 47ul,
    /* 4  */ 97ul,
    /* 5  */ 199ul,
    /* 6  */ 409ul,
    /* 7  */ 823ul,
    /* 8  */ 1741ul,
    /* 9  */ 3469ul,
    /* 10 */ 6949ul,
    /* 11 */ 14033ul,
    /* 12 */ 28411ul,
    /* 13 */ 57557ul,
    /* 14 */ 116731ul,
    /* 15 */ 236897ul,
    /* 16 */ 480881ul,
    /* 17 */ 976369ul,
    /* 18 */ 1982627ul,
    /* 19 */ 4026031ul,
    /* 20 */ 8175383ul,
    /* 21 */ 16601593ul,
    /* 22 */ 33712729ul,
    /* 23 */ 68460391ul,
    /* 24 */ 139022417ul,
    /* 25 */ 282312799ul,
    /* 26 */ 573292817ul,
    /* 27 */ 1164186217ul,
    /* 28 */ 2364114217ul,
    /* 29 */ 4294967291ul,
    /* 30 */ 8589934583ull,
    /* 31 */ 17179869143ull,
    /* 32 */ 34359738337ull,
    /* 33 */ 68719476731ull,
    /* 34 */ 137438953447ull,
    /* 35 */ 274877906899ull,
    /* 36 */ 549755813881ull,
    /* 37 */ 1099511627689ull,
    /* 38 */ 2199023255531ull,
    /* 39 */ 4398046511093ull,
    /* 40 */ 8796093022151ull,
    /* 41 */ 17592186044399ull,
    /* 42 */ 35184372088777ull,
    /* 43 */ 70368744177643ull,
    /* 44 */ 140737488355213ull,
    /* 45 */ 281474976710597ull,
    /* 46 */ 562949953421231ull,
    /* 47 */ 1125899906842597ull,
    /* 48 */ 2251799813685119ull,
    /* 49 */ 4503599627370449ull,
    /* 50 */ 9007199254740881ull,
    /* 51 */ 18014398509481951ull,
    /* 52 */ 36028797018963913ull,
    /* 53 */ 72057594037927931ull,
    /* 54 */ 144115188075855859ull,
    /* 55 */ 288230376151711717ull,
    /* 56 */ 576460752303423433ull,
    /* 57 */ 1152921504606846883ull,
    /* 58 */ 2305843009213693951ull,
    /* 59 */ 4611686018427387847ull,
    /* 60 */ 9223372036854775783ull,
    /* 61 */ 18446744073709551557ull,
};
//----------------------------------------------------------------- Function ---
/**
 * Returns the number of digits for given size_t number of up to 20 digits
 */
size_t count_digits(const size_t n)
{
	if (n < 10000000000) {
		// 10 or less
		if (n < 100000) {
			// 5 or less
			if (n < 100) {
				// 1 or 2
				if (n < 10)
					return 1;
				else
					return 2;
			}
			else {
				// 3 or 4 or 5
				if (n < 1000)
					return 3;
				else {
					// 4 or 5
					if (n < 10000)
						return 4;
					else
						return 5;
				}
			}
		}
		else {
			// 6 or more
			if (n < 10000000) {
				// 6 or 7
				if (n < 1000000)
					return 6;
				else
					return 7;
			}
			else {
				// 8 to 10
				if (n < 100000000)
					return 8;
				else {
					// 9 or 10
					if (n < 1000000000)
						return 9;
					else
						return 10;
				}
			}
		}
	}
	else {
		// 11 or more
		if (n < 1000000000000000) {
			// 5 or less
			if (n < 1000000000000) {
				// 1 or 2
				if (n < 100000000000)
					return 11;
				else
					return 12;
			}
			else {
				// 3 or 4 or 5
				if (n < 10000000000000)
					return 13;
				else {
					// 4 or 5
					if (n < 100000000000000)
						return 14;
					else
						return 15;
				}
			}
		}
		else {
			// 6 or more
			if (n < 100000000000000000) {
				// 6 or 7
				if (n < 10000000000000000)
					return 16;
				else
					return 17;
			}
			else {
				// 8 to 10
				if (n < 1000000000000000000)
					return 18;
				else {
					// 9 or 10
					if (n < 10000000000000000000ull)
						return 19;
					else
						return 20;
				}
			}
		}
	}
}

//----------------------------------------------------------------- Function ---
/**
 * Returns a multiplicative style hash for given key
 *
 * see : Linear congruential generator
 */
size_t hash_multiplicative(const char *key, const size_t seed)
{
	size_t hash = seed;
	// const size_t length = strlen(key);

	// for (size_t i = 0; i < length; i++) {
	// 	hash = seed * hash + key[i];
	// }
	while (*key) {
		hash = seed * hash + *key++;
	}

	return hash;
}

//----------------------------------------------------------------- Function ---
/**
 * Dump and Toy with prime numbers lut and hash
 */
// static void dump_prime_luts(void)
// {
// 	//----------------------------------------------------------- as numbers
// 	puts("\nprime numbers LUT used in cpp stl\n");

// 	for (size_t i = 0; i < ARRAY_LENGTH(prime_sizes); i++) {
// 		printf("prime #%lu \t digits : %lu \t => %lu\n",
// 		       i,
// 		       count_digits(prime_sizes[i]),
// 		       prime_sizes[i]);
// 	}

// 	//----------------------------------------------------------- as strings
// 	puts("\nas strings :\n");
// 	char prime_string[21];

// 	for (size_t i = 0; i < ARRAY_LENGTH(prime_sizes); i++) {
// 		snprintf(prime_string,
// 		         count_digits(prime_sizes[i]) + 1,
// 		         "%lu",
// 		         prime_sizes[i]);
// 		printf("prime #%lu \t digits : %lu \t | hash_mult : %016lx \t => %s\n",
// 		       i,
// 		       strlen(prime_string),
// 		       hash_multiplicative(prime_string, prime_numbers[7]),
// 		       prime_string);
// 	}
// }

//--------------------------------------------------------------------- MAIN ---
int main(void)
{
	// dump_prime_luts();

	//----------------------------------------------------------- hash tests
	const char *keys[] = {
	    "too",  "top",  "tor",  "tpp",  "a000", "a001", "a002", "a003", "a004",
	    "a005", "a006", "a007", "a008", "a009", "a010", "a",    "aa",   "aaa",
	    "add",  "sub",  "mul",  "div",  "mod",  "pow",  "max",  "min",  "+",
	    "-",    "*",    "/",    "%",    "^",    ">",    "<",    "head", "tail",
	    "list", "init", "eval", "join", "cons", "len",  "def",  "env"};

	//------------------------------------------------------- table size
	// find first prime_size bigger than the key list length times load factor
	size_t size_index     = 0;
	size_t load_factor    = 2;
	uint_fast16_t seed    = prime_numbers[20];
	size_t minimum_length = ARRAY_LENGTH(keys) * load_factor;
	while (prime_sizes[size_index] < minimum_length) {
		size_index++;
	};
#define MAP_SIZE 97

	KeyValuePair test_table[MAP_SIZE] = {{0, NULL, NULL}};

	printf(
	    "\nsize_index : %lu"
	    "\nminimum_length : %lu"
	    "\nprime_sizes[size_index] : %lu"
	    "\nseed : %lu\n",
	    size_index,
	    minimum_length,
	    prime_sizes[size_index],
	    seed);

	puts("\nhash tests :\n");
	size_t hash;
	Hash128 hash128;
	size_t bucket_index;

	for (size_t i = 0; i < ARRAY_LENGTH(keys); i++) {
		hash         = hash_multiplicative(keys[i], seed);
		hash128      = murmurhash3_x64(keys[i], strlen(keys[i]), seed);
		bucket_index = hash % MAP_SIZE;

		printf(
		    "#%lu \t murmurhash3_x64 : %016lx %016lx hash_mult : %016lx    "
		    "bucket : %lu    \t => %s",
		    i,
		    hash128.hi,
		    hash128.lo,
		    hash,
		    bucket_index,
		    keys[i]);

		if (test_table[bucket_index].key != NULL) {
			printf(" <== !!!! COLLISION !!!!\n");
		}
		else {
			putchar('\n');
			test_table[bucket_index].key =
			    XMALLOC(strlen(keys[i]) + 1,
			            "hash tests",
			            "test_table[bucket_index].key");
			strcpy(test_table[bucket_index].key, keys[i]);
			test_table[bucket_index].value = XMALLOC(
			    sizeof(i), "hash tests", "test_table[bucket_index].value");
			*((int *)test_table[bucket_index].value) = i;
		}
	}

	puts("\nhash table output :\n");
	for (size_t i = 0; i < ARRAY_LENGTH(test_table); i++) {
		if (test_table[i].key != NULL) {
			printf("#%lu \t %s\t: %d\n",
			       i,
			       test_table[i].key,
			       *((int *)test_table[i].value));
		}
	}

	//---------------------------------------------------------- cleanup
	for (size_t i = 0; i < ARRAY_LENGTH(test_table); i++) {
		if (test_table[i].key != NULL) {
			XFREE(test_table[i].key, "cleanup");
			XFREE((int *)test_table[i].value, "cleanup");
		}
	}

	return 0;
}