//------------------------------------------------------------------ SUMMARY ---
/**
 * Implement a hash map tuned for lispy / LispEnv
 *
 * Describe the plan for a hash map as I currently understand it :
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
 *   - [x] Implement basic multiplicative hash function
 *   - [x] Port murmur3 to C to suit your needs
 *   - [ ] Compare and select hash function
 *   - [ ] Consider secondary hash map for collisions
 *     + [ ] See :
 * http://courses.cs.vt.edu/~cs3114/Fall09/wmcquain/Notes/T17.PerfectHashFunctions.pdf
 *   - [ ] Consider quadratic probing for collisions
 * 	   + [ ] See :
 * https://stackoverflow.com/questions/22437416/best-way-to-resize-a-hash-table
 * 	   + [ ] See : https://github.com/jamesroutley/write-a-hash-table
 *
 *   - [x] Provide ** mode where user allocate, map points to
 * 	   + [x] Iterate through entry via doubly linked xor list
 *
 *   - [ ] Provide * mode where map is backed by contiguous array
 *     + [ ] Alloc map and backing array by chunks on creation and resize only
 *     + [ ] Map backing array index in (key, size_t value) hashtable !!
 *     + [ ] Resize by updating hashtable, not backing array
 *     + [ ] Do not discard backing array chunks, supplement them
 *     + [ ] Iterate through backing array, skip deleted
 *     + [ ] Allow replacing table in single atomic step by reassigning *
 *
 *   - [ ] Allocate collisions^2 slots in buckets
 *     + [ ] Hash with different function/seed until collision
 *     + [ ] Store function/seed in bucket
 *
 *   - [ ] Move to robin hood collision resolution
 *     + [ ] Discard insertion history list entirely
 *     + [ ] Avoid bound checking by overgrowing backing array by PROBE_LIMIT
 *
 *   - [ ] Move ValueDestructor to Hashmap instead of HashmapEntry
 *     + [ ] Consider putting it back only if you need mixed type values
 */

#include <math.h>   /* pow() */
#include <stddef.h> /* size_t */
#include <stdint.h> /* uint32_t, uint64_t */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <stdbool.h>

#include "ansi_esc.h"

#include <assert.h>
// #define DEBUG_MALLOC
#include "debug_xmalloc.h"

//------------------------------------------------------------ MAGIC NUMBERS ---
#define PROBE_LIMIT 8

//------------------------------------------------------------------- MACROS ---
#define ARRAY_LENGTH(_array) (sizeof(_array) / sizeof((_array)[0]))

//----------------------------------------------------- FORWARD DECLARATIONS ---
static inline size_t hash_tab(const unsigned char *key, const size_t *xor_seed)
    __attribute__((const, always_inline));

//------------------------------------------------------------- DECLARATIONS ---
/**
 * todo
 *   - [x] Consider a wrapper Hash type to deal with different hash bit depth
 */
// typedef Hash128 (*HashFunc)(const void *, const size_t, const uint32_t);
// typedef size_t (*HashFunc)(const char *, const size_t, const unsigned int);
typedef size_t (*HashFunc)(const unsigned char *, const size_t *);

typedef void *(*ValueConstructor)(const void *);
typedef void (*ValueDestructor)(void *);

typedef struct HashmapEntry {
	// bool is_collision;
	// size_t hash;
	int distance; /* to initial bucket */
	char *key;
	void *value;
} HashmapEntry;

typedef struct Hashmap {
	// const uint32_t seed;
	// HashFunc function;
	const size_t xor_seed[256];
	// ValueConstructor constructor;
	ValueDestructor destructor;
	unsigned int n;
	int probe_limit;
	size_t capacity;
	size_t count;
	size_t collisions;
	HashmapEntry *buckets;
} Hashmap;

typedef struct HashmapInit {
	// const uint32_t seed;
	// HashFunc function;
	size_t xor_seed[256];
	// ValueConstructor constructor;
	ValueDestructor destructor;
	unsigned int n;
	int probe_limit;
	size_t capacity;
	size_t count;
	size_t collisions;
	HashmapEntry *buckets;
} HashmapInit;

//----------------------------------------------------------------- Function ---
/**
 * Compute a tabulation style hash table size for given key, xor_seed table
 *   -> Hashmap index
 */
static inline size_t hash_tab(const unsigned char *key, const size_t *xor_seed)
{
	size_t hash = 0;

	while (*key) {
		hash ^= xor_seed[*key] ^ *key;
		key++;
	}

	return hash;
}

//----------------------------------------------------------------- Function ---
/**
 * Swap given entries
 *   -> nothing
 */
static inline void swap_entries(HashmapEntry *a, HashmapEntry *b)
{
	HashmapEntry temp;
	temp.distance = a->distance;
	temp.key      = a->key;
	temp.value    = a->value;

	a->distance = b->distance;
	a->key      = b->key;
	a->value    = b->value;

	b->distance = temp.distance;
	b->key      = temp.key;
	b->value    = temp.value;

	// a->distance ^= b->distance;
	// b->distance ^= a->distance;
	// a->distance ^= b->distance;
	// a->key = (char *) ((uintptr_t)a->key ^ (uintptr_t)b->key);
	// b->key = (char *) ((uintptr_t)a->key ^ (uintptr_t)b->key);
	// a->key = (char *) ((uintptr_t)a->key ^ (uintptr_t)b->key);
	// a->key = (void *) ((uintptr_t)a->value ^ (uintptr_t)b->value);
	// b->key = (void *) ((uintptr_t)a->value ^ (uintptr_t)b->value);
	// a->key = (void *) ((uintptr_t)a->value ^ (uintptr_t)b->value);
	return;
}
//----------------------------------------------------------------- Function ---
/**
 * Check if given HashmapEntry
 *   -> True if HashmapEntry.distance < 0
 */
static inline int is_empty(HashmapEntry *entry)
{
	return (entry->distance < 0);
}

//----------------------------------------------------------------- Function ---
/**
 * Create a new entry in given Hashmap for given key, value pair
 *
 *   Get hash for given key
 *   Locate bucket for that hash
 *   If bucket is empty
 * 	   Create new entry
 *     Insert entry in bucket
 *     Increment Hashmap entry count ( Check Hashmap fitness )
 * 		-> nothing
 *   Else !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *     If key found
 *       Delete value
 *       Replace with copy of given value
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *   Increment Hashmap entry count ( Check Hashmap fitness )
 *     -> nothing
 */
HashmapEntry *put_hashmap(Hashmap *hashmap, const char *key, void *value)
{
	assert(key != NULL);
	assert(value != NULL);

	size_t hash = hash_tab((unsigned char *)key, hashmap->xor_seed);
	printf("hash->[%lu]\n", hash);
	HashmapEntry *bucket = &hashmap->buckets[hash];

	/* Prepare temp entry*/
	HashmapEntry entry;
	entry.distance = 0;
	entry.key      = XMALLOC(strlen(key) + 1, "put_hashmap", "temp_entry.key");
	strcpy(entry.key, key);
	entry.value = value;

	for (; entry.distance < hashmap->probe_limit; entry.distance++, bucket++) {
		/* Empty bucket */
		if (bucket->key == NULL) {
			bucket->key      = entry.key;
			bucket->value    = entry.value;
			bucket->distance = entry.distance;
			// check_hashmap(hashmap);
			hashmap->count++;
			if (entry.distance > 0) {
				hashmap->collisions++;
			}
			return bucket;
		}
		else {
			/* Rich bucket */
			if (entry.distance > bucket->distance) {
				swap_entries(&entry, bucket);
			}
		}
	}

	/* Past probe limit, trigger resize */
	XFREE(entry.key, "past probe limit");
	return NULL;
}

//----------------------------------------------------------------- Function
//---
/**
 * Free given HashmapEntry
 *   -> nothing
 */
static inline void destroy_entry(HashmapEntry *entry)
//, ValueDestructor destructor)
{
	XFREE(entry->key, "destroy_entry");
	// destructor(entry->value);
	XFREE(entry->value, "destroy_entry");
	// free(entry);
	return;
}
//----------------------------------------------------------------- Function
//---
/**
 * Locate HasmapEntry for given key in given Hashmap
 * If found
 *   Remove HashmapEntry from insertion list
 *   Free HashmapEntry
 *   -> nothing
 */
// void delete_hashmap_entry(Hashmap *hashmap, const char *key)
// {
// 	const Hash128 hash = hashmap->function(key, strlen(key), hashmap->seed);
// 	const size_t index = mod_hash128(hash, hashmap->n);
// 	destroy_entry(hashmap->buckets[index]);
// }

//----------------------------------------------------------------- Function
//---
/**
 * Free given Hashmap
 *   -> nothing
 */
void delete_hashmap(Hashmap *hashmap)
{
	for (size_t i = 0; i < hashmap->capacity; i++) {

		if (hashmap->buckets[i].key == NULL) {
			continue;
		}
		printf("%s\n", hashmap->buckets[i].key);
		destroy_entry(&hashmap->buckets[i]); //, hashmap->destructor);
	}
	XFREE(hashmap->buckets, "delete_hashmap");
	XFREE(hashmap, "delete_hashmap");
}
//----------------------------------------------------------------- Function
//---
/**
 * Create Hashmap of capacity equal to 2^(given n),  with given seed
 *
 *   Init a dummy xor_seed
 *   Init a dummy HashmapInit
 * 	 Copy dummy xor_seed in dummy HashmapInit
 *   Allocate a Hashmap
 *   Copy dummy HashmapInit to Hashmap
 *   Allocate Hashmap buckets
 *   Init Hashmap buckets to NULL
 *   Init insertion list
 *     -> pointer to HashTable
 *
 * todo
 *   - [ ] Replace parameter n with requested_capacity
 *     + [ ] Compute next 2^(n) - 1 from requested_capacity
 *     + [ ] Store n
 */
Hashmap *new_hashmap(const unsigned int n,
                     //  ValueConstructor constructor,
                     ValueDestructor destructor)
{
	/* Advertised capacity */
	/* current tab_hash implementation requires a mininum of 256 slots */
	const size_t capacity = (n < 8) ? (1u << 8) : (1u << n);
	//---------------------------------------------------- xor_seed init
	/**
	 * todo
	 *   + [ ] Do not discard most of each rand() with % capacity
	 *     - [ ] Spread enough rand() to fill the table splitting according
	 * to
	 *           capacity
	 */
	size_t xor_seed_init[256];
	for (size_t i = 0; i < 256; i++) {
		xor_seed_init[i] = rand() % capacity;
	}

	//----------------------------------------------------- Hashmap init
	HashmapInit hashmap_init = {
	    {0}, destructor, n, PROBE_LIMIT, capacity, 0, 0, NULL};
	memcpy(hashmap_init.xor_seed, xor_seed_init, sizeof(xor_seed_init));

	Hashmap *new_hashmap =
	    XMALLOC(sizeof(Hashmap), "new_hashmap", "new_hashmap");
	memcpy(new_hashmap, &hashmap_init, sizeof(Hashmap));

	// HashmapEntry bucket_init = {0, NULL, NULL};
	new_hashmap->buckets =
	    XMALLOC(sizeof(HashmapEntry) * capacity + PROBE_LIMIT,
	            "new_hashmap",
	            "new_hashmap->buckets");
	if (new_hashmap->buckets == NULL) {
		return NULL;
	}

	memset(
	    new_hashmap->buckets, 0, sizeof(HashmapEntry) * capacity + PROBE_LIMIT);

	return new_hashmap;
}
//----------------------------------------------------------------- Function
//---
/**
 * Compute the expected number of entries per bucket for given number of
 * keys,
 * buckets
 *   -> expected number of entries per bucket
 */
double expected_entries_per_bucket(size_t keys, size_t buckets)
{
	return (double)keys / (double)buckets;
}
//----------------------------------------------------------------- Function
//---
/**
 * Compute the expected number of empty buckets for given number of keys,
 * buckets
 *   -> expected number of empty buckets
 */
double expected_empty_buckets(size_t keys, size_t buckets)
{
	return (double)buckets * pow(1 - (1 / (double)buckets), (double)keys);
}
//----------------------------------------------------------------- Function
//---
/**
 * Compute the expected number of filled buckets for given number of keys,
 * buckets
 *   -> expected number of filled buckets
 */
double expected_filled_buckets(size_t keys, size_t buckets)
{
	return (double)buckets - expected_empty_buckets(keys, buckets);
}
//----------------------------------------------------------------- Function
//---
/**
 * Compute the expected number of collisions for given number of keys,
 * buckets
 *   -> expected number of collisions
 */
double expected_collisions(size_t keys, size_t buckets)
{
	return (double)keys - expected_filled_buckets(keys, buckets);
}

//----------------------------------------------------------------- Function
//---
/**
 * Pretty print xor_seed for given Hashmap
 *   -> nothing
 */
void print_xor_seed(Hashmap *hashmap)
{
	puts("xor_seed            :");
	for (size_t i = 0; i < 256; i++) {
		printf("%02lx ", hashmap->xor_seed[i]);
		if (((i + 1) % 16) == 0) {
			putchar('\n');
		}
	}
	putchar('\n');

	return;
}
//----------------------------------------------------------------- Function
//---
/**
 * Dump given Hashmap content
 *   -> nothing
 *
 * todo
 *   - [x] Consider a insertion ordered double-linked list of HashmapEntry
 *   - [ ] Implement list iterator
 *     + [ ] See http://rosettacode.org/wiki/Doubly-linked_list/Traversal#C
 */
void dump_hashmap(Hashmap *hashmap)
{
	size_t empty_bucket = 0;

	for (size_t i = 0; i < hashmap->capacity; i++) {
		if (hashmap->buckets[i].key == NULL) {
			empty_bucket++;
			continue;
		}
		printf("\x1b[10%dmhashmap->\x1b[30mbucket[%lu]\x1b[0m>>%d\n",
		       hashmap->buckets[i].distance,
		       i,
		       hashmap->buckets[i].distance);

		// printf("\t %016lx : %s\n",
		printf("\t        %lu : %s\n",
		       hash_tab((unsigned char *)hashmap->buckets[i].key,
		                hashmap->xor_seed),
		       hashmap->buckets[i].key);
	}

	printf("EV entries / bucket = %f\n",
	       expected_entries_per_bucket(hashmap->count, hashmap->capacity));
	printf("EV collisions = %f \t-> %f%%\n",
	       expected_collisions(hashmap->count, hashmap->capacity),
	       expected_collisions(hashmap->count, hashmap->capacity) /
	           (double)hashmap->count * 100);
	printf("EV empty = %f \t-> %f%%\n",
	       expected_empty_buckets(hashmap->count, hashmap->capacity),
	       expected_empty_buckets(hashmap->count, hashmap->capacity) /
	           (double)hashmap->capacity * 100);

	printf("hashmap->n          : %u\n", hashmap->n);
	printf("hashmap->probe_lim  : %d\n", hashmap->probe_limit);
	printf("hashmap->capacity   : %lu\n", hashmap->capacity);
	printf("hashmap->count      : %lu \t-> %f%%\n",
	       hashmap->count,
	       (double)hashmap->count / (double)hashmap->capacity * 100);
	printf("hashmap->collisions : %lu \t-> %lf%%\n",
	       hashmap->collisions,
	       //    (float)hashmap->collisions / (float)hashmap->capacity,
	       (double)hashmap->collisions / (double)hashmap->count * 100);
	printf("empty_buckets       : %lu \t-> %f%%\n",
	       empty_bucket,
	       (double)empty_bucket / (double)hashmap->capacity * 100);

	print_xor_seed(hashmap);
}

//--------------------------------------------------------------------- MAIN
//---
int main(void)
{
	// uint32_t seed = 31;
	// todo
	//   - [ ] Confirm tab_hash should NOT work for n < 8 as it is
	size_t n = 8;
	// Hashmap *hashmap = new_hashmap(n, seed, hash_fimur_reduce);

	fputs(FG_BRIGHT_BLUE REVERSE
	      "Table size is 2^n. Enter n ( default n=8 ) " FG_BRIGHT_BLUE
	      " ? " RESET,
	      stdout);
	scanf("%lu", &n);

	Hashmap *hashmap = new_hashmap(n, free);
//-------------------------------------------------------------------- setup
#define KEYPOOL_SIZE 32
	char random_keys[KEYPOOL_SIZE] = {'\0'};
	size_t test_count              = (1 << n) - 1; // 1 << (n - 1);
	char key[256];
	char *dummy_value = NULL;
	// HashmapEntry *temp_entry;

	for (size_t k = 0; k < test_count; k++) {
		for (size_t i = 0; i < KEYPOOL_SIZE - 1; i++) {
			random_keys[i] = (char)(rand() % 95 + 0x20);
			// putchar(random_keys[i]);
		}
		// putchar('\n');

		dummy_value = strdup(&random_keys[rand() % (KEYPOOL_SIZE - 1)]);
		printf(
		    "[%lu]key   : %s\n"
		    "[%lu]value : %s\n",
		    k,
		    &random_keys[rand() % (KEYPOOL_SIZE - 1)],
		    k,
		    dummy_value);

		put_hashmap(
		    hashmap, &random_keys[rand() % (KEYPOOL_SIZE - 1)], dummy_value);
		// if (temp_entry == NULL) {
		// 	XFREE(dummy_value, "main : setup");
		// }
	}

	//----------------------------------------------------------- input loop
	// dump_hashmap(hashmap);

	for (;;) {
		fputs("\x1b[102m > \x1b[0m", stdout);
		scanf("%s", key);

		if ((strcmp(key, "exit")) == 0) {
			break;
		}

		if ((strcmp(key, "dump")) == 0) {
			dump_hashmap(hashmap);
			continue;
		}

		if ((strcmp(key, "seed")) == 0) {
			print_xor_seed(hashmap);
			continue;
		}
		//-------------------------------------------- lookup prototype
		// const size_t index =
		//     hash_tab((unsigned char *)key, hashmap->xor_seed);
		put_hashmap(hashmap, key, strdup(key));
		// if (temp_entry == NULL) {
		// 	XFREE(dummy_value, "main : input loop");
		// }
	}

	//-------------------------------------------------------------- cleanup
	if (dummy_value != NULL) {
		XFREE(dummy_value, "main : input loop");
	}
	delete_hashmap(hashmap);
	return 0;
}

//----------------------------------------------------------------- Function ---
/**
 * Compute modulus division by 2^n - 1 for given integer dividend
 *   -> Remainder
 */
// static inline size_t mod_2n1(uint_fast64_t dividend, const unsigned int n)
// {
// 	const uint_fast64_t divisor = (1u << n) - 1;
// 	uint64_t remainder;

// 	for (remainder = dividend; dividend > divisor; dividend = remainder) {
// 		for (remainder = 0; dividend; dividend >>= n) {
// 			remainder += dividend & divisor;
// 		}
// 	}
// 	return remainder == divisor ? 0 : remainder;
// }

//----------------------------------------------------------------- Function ---
/**
 * Compute modulus division by 2^n - 1 for given Hash128
 *   -> Remainder
 */
// static inline size_t mod_hash128(const Hash128 hash, const unsigned int n)
// {
// 	const uint32_t h1 = (hash.hi & 0xFFFFFFFF00000000ul) >> 32;
// 	const uint32_t h2 = (hash.hi & 0x00000000FFFFFFFFul);
// 	const uint32_t h3 = (hash.lo & 0xFFFFFFFF00000000ul) >> 32;
// 	const uint32_t h4 = (hash.lo & 0x00000000FFFFFFFFul);

// 	return mod_2n1(
// 	    (mod_2n1((mod_2n1((mod_2n1(h1, n) << 32) + h2, n) << 32) + h3, n)
// 	     << 32) +
// 	        h4,
// 	    n);
// }

//----------------------------------------------------------------- Function ---
/**
 * Compute a rotated multiplicative style hash mapped to given n power of 2
 * table size for given key
 *   -> Hashmap index
 */
// static inline size_t hash_fimur_reduce(const char *key,
//                                        const size_t seed,
//                                        const unsigned int n)
// {
// 	size_t hash = seed;

// 	while (*key) {
// 		hash *= 11400714819323198485llu;
// 		hash *= *(key++) << 15;
// 		hash = (hash << 7) | (hash >> (32 - 7));
// 	}

// 	return hash >> (64 - n);
// }
// struct HashmapEntry;
// struct Hashmap;
// typedef struct HashmapEntry HashmapEntry;
// typedef struct Hashmap Hashmap;

// static inline size_t mod_2n1(uint_fast64_t dividend, const unsigned int n)
//     __attribute__((const, always_inline));
// // static size_t mod_hash128(const Hash128 hash, const unsigned int n)
// //     __attribute__((const, always_inline));
// static inline size_t hash_fimur_reduce(const char *key,
//                                        const size_t seed,
//                                        const unsigned int n)
//     __attribute__((const, always_inline));