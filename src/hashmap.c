//------------------------------------------------------------------ SUMMARY ---
/**
 * Implement a hash map tuned for lispy / LispEnv
 *
 * todo
 *   - [ ] Provide * mode where map is backed by contiguous array
 *     + [x] Alloc map and backing array by chunks on creation and resize only
 *     + [x] Map backing array index in (key, size_t value) hashtable !!
 *     + [ ] Resize by updating hashtable, not backing array
 *     + [ ] Do not discard backing array chunks, supplement them
 *     + [ ] Iterate through backing array, skip deleted
 *     + [ ] Allow replacing table in single atomic step by reassigning *
 *
 *   - [ ] Move to robin hood collision resolution
 *     + [x] Discard insertion history list entirely
 *     + [x] Avoid probe bound checking by overgrowing backing array by
 * PROBE_LIMIT
 *
 *   - [ ] Store bucket states in a separate array, 1 byte per bucket
 *     + [ ] Restrict PROBE_LIMIT to a multiple of that chunk size
 *     + [ ] Probe by vector friendly chunks
 *     + [ ] Use a overgrown xor_seed to match (capacity + PROBE_LIMIT^2)
 *       * [ ] Use MSpart matched to Capacity
 *       * [ ] Use LSpart matched to PROBE_LIMIT SQUARED
 *             you are not using it to index, but to check for match
 *       * [ ] Find group?? for given key with first
 *       * [ ] Probe inside group with second
 *
 *   - [ ] Implement delete procedure using backward shift approach
 */

#include <math.h>   /* pow() */
#include <stddef.h> /* size_t */
#include <stdint.h> /* uint32_t, uint64_t */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* __rdtsc() _mm_set1_epi8 _mm_movemask_epi8 _mm_cmpeq_epi8 */
#ifdef _MSC_VER
#include <intrin.h> 
#else
// #include <x86intrin.h>
#include <immintrin.h>
#endif

#include "ansi_esc.h"

#include <assert.h>
// #define DEBUG_MALLOC
#include "debug_xmalloc.h"

//------------------------------------------------------------ MAGIC NUMBERS ---
#define PROBE_LIMIT 16

//------------------------------------------------------------------- MACROS ---
#define ARRAY_LENGTH(_array) (sizeof(_array) / sizeof((_array)[0]))

//----------------------------------------------------- FORWARD DECLARATIONS ---
static inline size_t hash_tab(const unsigned char *key, const size_t *xor_seed)
    __attribute__((const, always_inline));

static inline size_t hash_index(const size_t hash)
    __attribute__((const, always_inline));

static inline ctrl_bit hash_meta(const size_t hash)
    __attribute__((const, always_inline));

void stats_hashmap(struct hmap *hashmap);
//------------------------------------------------------------- DECLARATIONS ---
typedef size_t (*HashFunc)(const unsigned char *, const size_t *);

typedef void *(*ValueConstructor)(const void *);
typedef void (*ValueDestructor)(void *);
// typedef void (*ValueDestructor)(void *, const char *, const char *);

typedef enum ctrl_bit {
	EMPTY     = -128,
	DELETED   = -2,
	TOMBSTONE = -1,
} ctrl_bit;

struct hmap_stats {
	size_t hashes_tally_or;
	size_t hashes_tally_and;
	size_t put_count;
	size_t swap_count;
	size_t putfail_count;
	size_t del_count;
	size_t find_count;
	size_t collision_count;
};

struct hmap_buckets {
	// Try SoA setup for buckets :
	unsigned char *metas;
	int *distances;
	char **keys;
	void **values;
};

struct hmap {
	const size_t xor_seed[256];
	ValueDestructor destructor;
	unsigned int n;
	int probe_limit;
	size_t capacity;
	size_t count;
	struct hmap_stats stats;
	struct hmap_buckets buckets;
};

struct hmap_init {
	size_t xor_seed[256];
	ValueDestructor destructor;
	unsigned int n;
	int probe_limit;
	size_t capacity;
	size_t count;
	struct hmap_stats stats;
	struct hmap_buckets buckets;
};

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

static inline size_t hash_index(const size_t hash) { return hash >> 8; }

static inline ctrl_bit hash_meta(const size_t hash) { return hash & 0xFF; }
//----------------------------------------------------------------- Function ---
/**
 * Swap given entries
 *   -> nothing
 */
static inline void swap_entries(struct hmap *hashmap, size_t a, size_t b)
{
	unsigned char tmp_meta;
	int tmp_distance;
	char *tmp_key;
	void *tmp_value;

	tmp_meta     = hashmap->buckets.metas[a];
	tmp_distance = hashmap->buckets.distances[a];
	tmp_key      = hashmap->buckets.keys[a];
	tmp_value    = hashmap->buckets.values[a];

	hashmap->buckets.metas[a]     = hashmap->buckets.metas[b];
	hashmap->buckets.distances[a] = hashmap->buckets.distances[b];
	hashmap->buckets.keys[a]      = hashmap->buckets.keys[b];
	hashmap->buckets.values[a]    = hashmap->buckets.values[b];

	hashmap->buckets.metas[a]     = tmp_meta;
	hashmap->buckets.distances[a] = tmp_distance;
	hashmap->buckets.keys[a]      = tmp_key;
	hashmap->buckets.values[a]    = tmp_value;

	/**
	 * todo
	 *   - [ ] Check if this compile to something different
	 */
	// tmp_meta                  = hashmap->buckets.metas[a];
	// hashmap->buckets.metas[a] = hashmap->buckets.metas[b];
	// hashmap->buckets.metas[a] = tmp_meta;

	// tmp_distance                  = hashmap->buckets.distances[a];
	// hashmap->buckets.distances[a] = hashmap->buckets.distances[b];
	// hashmap->buckets.distances[a] = tmp_distance;

	// tmp_key                  = hashmap->buckets.keys[a];
	// hashmap->buckets.keys[a] = hashmap->buckets.keys[b];

	// hashmap->buckets.keys[a]   = tmp_key;
	// tmp_value                  = hashmap->buckets.values[a];
	// hashmap->buckets.values[a] = hashmap->buckets.values[b];
	// hashmap->buckets.values[a] = tmp_value;

	return;
}
//----------------------------------------------------------------- Function ---
/**
 * Check if given HashmapEntry
 *   -> True if HashmapEntry.distance < 0
 */
static inline int is_empty(struct hmap *hashmap, size_t entry)
{
	return (hashmap->buckets.distances[entry] < 0);
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
size_t put_hashmap(struct hmap *hashmap, const char *key, void *value)
{
	assert(key != NULL);
	assert(value != NULL);

	// todo - [ ] Split find logic to find function !
	size_t hash = hash_tab((unsigned char *)key, hashmap->xor_seed);
	hashmap->stats.hashes_tally_or |= hash;
	hashmap->stats.hashes_tally_and &= hash;

	// printf("hash->[%lu]\n", hash);
	// HashmapEntry *bucket = &hashmap->buckets[hash];
	/**
	 * todo
	 *   - [ ] Check if this compile to something different
	 */
	// unsigned char *meta;
	// int * distance;
	// char **key;
	// void **value;

	unsigned char entry_meta = hash_meta(hash);
	size_t entry_index       = hash_index(hash);

	__m128i filter = _mm_set1_epi8(entry_meta);
	int match_mask = _mm_movemask_epi8(_mm_cmpeq_epi8(filter), );

	/* Prepare temp entry*/
	struct hmap_buckets entry;
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

			/* Solved collision */
			if (entry.distance > 0) {
				hashmap->stats.collision_count++;
			}
			hashmap->stats.put_count++;
			return bucket;
		}
		else {
			/* Key exist */
			// todo
			//   - [ ] Probe for existing key
			//   - [ ] Make sure comparison is necessary and cheap
			if (strcmp(entry.key, bucket->key) == 0) {
				XFREE(entry.key, "duplicate key");
				hashmap->destructor(
				    bucket->value); //, "bucket->value", "destructor");
				bucket->value = entry.value;
				hashmap->stats.put_count++;
				return bucket;
			}

			// /* Rich bucket */
			// if (entry.distance > bucket->distance) {
			// 	swap_entries(&entry, bucket);
			// 	hashmap->stats.swap_count++;
			// }
		}
	}

	/* Past probe limit, trigger resize */
	XFREE(entry.key, "past probe limit");
	hashmap->stats.putfail_count++;
	return NULL;
}

//----------------------------------------------------------------- Function ---
/**
 * Free given HashmapEntry
 *   -> nothing
 */
static inline void destroy_entry(HashmapEntry *entry)
//, ValueDestructor destructor)
{
	XFREE(entry->key, entry->key);
	// destructor(entry->value);
	XFREE(entry->value, "destroy_entry");
	// free(entry);
	return;
}
//----------------------------------------------------------------- Function ---
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

//----------------------------------------------------------------- Function ---
/**
 * Free given Hashmap
 *   -> nothing
 */
void delete_hashmap(Hashmap *hashmap)
{
	for (size_t i = 0; i < (hashmap->capacity + hashmap->probe_limit); i++) {

		if (hashmap->buckets[i].key == NULL) {
			continue;
		}
		// printf("%s\n", hashmap->buckets[i].key);
		destroy_entry(&hashmap->buckets[i]); //, hashmap->destructor);
		hashmap->stats.del_count++;
	}
	stats_hashmap(hashmap);
	XFREE(hashmap->buckets, "delete_hashmap");
	XFREE(hashmap, "delete_hashmap");
}
//----------------------------------------------------------------- Function ---
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
	size_t xor_seed_init[256];
	srand(__rdtsc());
	for (size_t i = 0; i < 256; i++) {
		// % capacity is okay-ish as long as its a power of 2
		// Expect skewed distribution otherwise
		xor_seed_init[i] = rand() % (capacity); // + PROBE_LIMIT * PROBE_LIMIT);
		// xor_seed_init[i-1] = hash_fimur_reduce((char *)&i, 31, n);
	}

	//----------------------------------------------------- Hashmap init
	HashmapStat stats_init   = {0, SIZE_MAX, 0, 0, 0, 0, 0, 0};
	HashmapInit hashmap_init = {
	    {0}, destructor, n, PROBE_LIMIT, capacity, 0, stats_init, NULL};
	memcpy(hashmap_init.xor_seed, xor_seed_init, sizeof(xor_seed_init));

	Hashmap *new_hashmap =
	    XMALLOC(sizeof(Hashmap), "new_hashmap", "new_hashmap");
	memcpy(new_hashmap, &hashmap_init, sizeof(Hashmap));

	// HashmapEntry bucket_init = {0, NULL, NULL};
	new_hashmap->buckets =
	    XMALLOC(sizeof(HashmapEntry) * (capacity + PROBE_LIMIT),
	            "new_hashmap",
	            "new_hashmap->buckets");
	if (new_hashmap->buckets == NULL) {
		return NULL;
	}

	memset(new_hashmap->buckets,
	       0,
	       sizeof(HashmapEntry) * (capacity + PROBE_LIMIT));

	return new_hashmap;
}
//----------------------------------------------------------------- Function ---
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
//----------------------------------------------------------------- Function ---
/**
 * Compute the expected number of empty buckets for given number of keys,
 * buckets
 *   -> expected number of empty buckets
 */
double expected_empty_buckets(size_t keys, size_t buckets)
{
	return (double)buckets * pow(1 - (1 / (double)buckets), (double)keys);
}
//----------------------------------------------------------------- Function ---
/**
 * Compute the expected number of filled buckets for given number of keys,
 * buckets
 *   -> expected number of filled buckets
 */
double expected_filled_buckets(size_t keys, size_t buckets)
{
	return (double)buckets - expected_empty_buckets(keys, buckets);
}
//----------------------------------------------------------------- Function ---
/**
 * Compute the expected number of collisions for given number of keys,
 * buckets
 *   -> expected number of collisions
 */
double expected_collisions(size_t keys, size_t buckets)
{
	return (double)keys - expected_filled_buckets(keys, buckets);
}

//----------------------------------------------------------------- Function ---
/**
 * Pretty print xor_seed for given Hashmap
 *   -> nothing
 */
void print_xor_seed(Hashmap *hashmap)
{
	puts("xor_seed            :");
	for (size_t i = 0; i < 256; i++) {
		printf("%0*lx ", (int)ceil((hashmap->n / 4.0)), hashmap->xor_seed[i]);
		if (((i + 1) % 16) == 0) {
			putchar('\n');
		}
	}
	putchar('\n');

	return;
}

//----------------------------------------------------------------- Function ---
/**
 * Print given Hashmap stats
 *   -> nothing
 */
void stats_hashmap(Hashmap *hashmap)
{
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
	printf("hashmap->capacity   : %lu / %lu\n",
	       hashmap->capacity,
	       hashmap->capacity + hashmap->probe_limit);
	printf("hashmap->count      : %lu \t-> %f%%\n",
	       hashmap->count,
	       (double)hashmap->count / (double)hashmap->capacity * 100);
	printf("hashmap->stats.collision_count  : %lu \t-> %lf%%\n",
	       hashmap->stats.collision_count,
	       (double)hashmap->stats.collision_count / (double)hashmap->count *
	           100);

	printf("hashmap->stats.hashes_tally_or  : %lu\n",
	       hashmap->stats.hashes_tally_or);
	printf("hashmap->stats.hashes_tally_and : %lu\n",
	       hashmap->stats.hashes_tally_and);
	printf("hashmap->stats.put_count        : %lu\n", hashmap->stats.put_count);
	printf("hashmap->stats.swap_count       : %lu\n",
	       hashmap->stats.swap_count);
	printf("hashmap->stats.putfail_count    : %lu\n",
	       hashmap->stats.putfail_count);
	printf("hashmap->stats.del_count        : %lu\n", hashmap->stats.del_count);
	printf("hashmap->stats.find_count       : %lu\n",
	       hashmap->stats.find_count);
	printf("hashmap->stats.collision_count  : %lu\n",
	       hashmap->stats.collision_count);
}
//----------------------------------------------------------------- Function ---
/**
 * Dump given Hashmap content
 *   -> nothing
 *
 * todo
 *   - [ ] Implement list iterator
 *     + [ ] See http://rosettacode.org/wiki/Doubly-linked_list/Traversal#C
 */
void dump_hashmap(Hashmap *hashmap)
{
	size_t empty_bucket = 0;

	for (size_t i = 0; i < (hashmap->capacity + hashmap->probe_limit); i++) {
		if (hashmap->buckets[i].key == NULL) {
			empty_bucket++;
			printf("\x1b[100mhashmap->\x1b[30mbucket[%lu]>> EMPTY <<\x1b[0m\n",
			       i);
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

	printf("empty_buckets       : %lu \t-> %f%%\n",
	       empty_bucket,
	       (double)empty_bucket / (double)hashmap->capacity * 100);

	stats_hashmap(hashmap);

	print_xor_seed(hashmap);
}

//--------------------------------------------------------------------- MAIN ---
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

	Hashmap *hashmap = new_hashmap(n, free); // xfree);
//-------------------------------------------------------------------- setup
#define KEYPOOL_SIZE 32
	char random_keys[KEYPOOL_SIZE] = {'\0'};
	size_t test_count              = (1 << (n - 2)) - 1; // 1 << (n - 1);
	char key[256];
	char *dummy_value = NULL;

	srand(__rdtsc());

	for (size_t k = 0; k < test_count; k++) {
		for (size_t i = 0; i < KEYPOOL_SIZE - 1; i++) {
			random_keys[i] = (char)(rand() % 95 + 0x20); // % 26 + 0x61);
			// putchar(random_keys[i]);
		}
		// putchar('\n');

		dummy_value = strdup(&random_keys[rand() % (KEYPOOL_SIZE - 1)]);
		// printf(
		//     "[%lu]key   : %s\n"
		//     "[%lu]value : %s\n",
		//     k,
		//     &random_keys[rand() % (KEYPOOL_SIZE - 1)],
		//     k,
		//     dummy_value);

		if (put_hashmap(hashmap,
		                &random_keys[rand() % (KEYPOOL_SIZE - 1)],
		                dummy_value) == NULL) {
			XFREE(dummy_value, "main : setup");
		}
	}

	//----------------------------------------------------------- input loop
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
		if ((strcmp(key, "stats")) == 0) {
			stats_hashmap(hashmap);
			continue;
		}

		if ((strcmp(key, "add")) == 0) {
			printf("todo\n");
			continue;
		}
		//-------------------------------------------- lookup prototype
		// const size_t index =
		//     hash_tab((unsigned char *)key, hashmap->xor_seed);
		dummy_value = strdup(key);
		if (put_hashmap(hashmap, key, dummy_value) == NULL) {
			XFREE(dummy_value, "main : setup");
		}
	}

	//-------------------------------------------------------------- cleanup
	delete_hashmap(hashmap);
	return 0;
}

// static inline size_t hash_fimur_reduce(const char *key,
//                                        const size_t seed,
//                                        const unsigned int n)
//    __attribute__((const, always_inline));

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