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
 *           PROBE_LIMIT
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
 *   - [ ] Use fast range instead of modulo if tab_hash isn't fit, bitmask if
 *         desperate
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
// #include <immintrin.h>
#include <x86intrin.h>
#endif

#include "ansi_esc.h"

#include <assert.h>
// #define DEBUG_HMAP

#include "debug_xmalloc.h"

#ifdef DEBUG_HMAP
#define DEBUG_MALLOC
#endif /* DEBUG_HMAP */
//------------------------------------------------------------ MAGIC NUMBERS ---
#define PROBE_LIMIT 16

//------------------------------------------------------------------- MACROS ---
#define ARRAY_LENGTH(_array) (sizeof(_array) / sizeof((_array)[0]))

//------------------------------------------------------------- DECLARATIONS ---
typedef signed char ctrl_byte;

enum ctrl_bit { CTRL_EMPTY = -128, CTRL_DELETED = -1 };

// typedef size_t (*HashFunc)(const unsigned char *, const size_t *);

struct hmap_allocator {
	void *(*malloc)(size_t size);
	void (*free)(void *);
	// typedef void (*ValueDestructor)(void *, const char *, const char *);
};

#ifdef DEBUG_HMAP
struct hmap_stats {
	size_t hashes_tally_or;
	size_t hashes_tally_and;
	size_t put_count;
	size_t collision_count;
	size_t hashes_ctrl_collision_count;
	size_t swap_count;
	size_t putfail_count;
	size_t del_count;
	size_t find_count;
};
#endif /* DEBUG_HMAP */

struct hmap_buckets {
	// Try SoA setup for buckets :
	ctrl_byte *metas;
	int *distances;
	char **keys;
	void **values;
};

struct hmap {
	const size_t xor_seed[256];
	struct hmap_buckets buckets;
	const struct hmap_allocator allocator;
	unsigned int n;
	int probe_limit;
	size_t capacity;
	size_t count;
#ifdef DEBUG_HMAP
	struct hmap_stats stats;
#endif /* DEBUG_HMAP */
};

struct hmap_init {
	size_t xor_seed[256];
	struct hmap_buckets buckets;
	struct hmap_allocator allocator;
	unsigned int n;
	int probe_limit;
	size_t capacity;
	size_t count;
#ifdef DEBUG_HMAP
	struct hmap_stats stats;
#endif /* DEBUG_HMAP */
};

//----------------------------------------------------- FORWARD DECLARATIONS ---
static inline size_t hash_tab(const unsigned char *key, const size_t *xor_seed)
    __attribute__((const, always_inline));

static inline size_t hash_index(const size_t hash)
    __attribute__((const, always_inline));

static inline ctrl_byte hash_meta(const size_t hash)
    __attribute__((const, always_inline));

// inline size_t hmap_find(const struct hmap *hashmap, const char *key)
//     __attribute__((const, always_inline));

static inline void destroy_entry(struct hmap *hashmap, size_t entry)
    __attribute__((always_inline));

// void stats_hashmap(const struct hmap *hashmap);

//----------------------------------------------------------------- Function ---
/**
 * Compute a tabulation style hash for given key, xor_seed table
 *   -> hmap index
 *
 * todo
 *   - [ ] Assess hash function fitness
 *     + [x] Track stuck bits with hashes_tally_or, hashes_tally_and
       + [ ] Track control group collisions likelyhood with
             hashes_ctrl_collision_count
       + [ ] Look at SMHasher for more ways to assess fitness
         * [ ] See : https://github.com/aappleby/smhasher/wiki/SMHasher
 */
static inline size_t hash_tab(const unsigned char *key, const size_t *xor_seed)
{
	size_t hash = 0;

	while (*key) {
		hash ^= xor_seed[(*key + hash) & 0xFF] ^ *key;
#ifdef DEBUG_HMAP
		printf(
		    "partial hash : %lu\n"
		    "char         : %c\n"
		    "xor[char]    : %lu\n\n",
		    hash,
		    (char)*key,
		    xor_seed[*key]);
#endif /* DEBUG_HMAP */
		key++;
	}

	return hash;
}
// static inline size_t hash_tab_sse(const unsigned char *key,
//                                   const size_t *xor_seed)
// {
// 	size_t hash = 0;
// 	/**
// 	 * todo
// 	 *   + [ ] Try N bytes at a time sse version of hash_tab
// 	 */
// 	return hash;
// }

static inline size_t hash_index(const size_t hash) { return hash >> 7; }

static inline ctrl_byte hash_meta(const size_t hash) { return hash & 0x7F; }
// //----------------------------------------------------------------- Function
// ---
// /**
//  * Swap given entries
//  *   -> nothing
//  */
// static inline void swap_entries(struct hmap *hashmap, size_t a, size_t b)
// {
// 	unsigned char tmp_meta;
// 	int tmp_distance;
// 	char *tmp_key;
// 	void *tmp_value;

// 	tmp_meta     = hashmap->buckets.metas[a];
// 	tmp_distance = hashmap->buckets.distances[a];
// 	tmp_key      = hashmap->buckets.keys[a];
// 	tmp_value    = hashmap->buckets.values[a];

// 	hashmap->buckets.metas[a]     = hashmap->buckets.metas[b];
// 	hashmap->buckets.distances[a] = hashmap->buckets.distances[b];
// 	hashmap->buckets.keys[a]      = hashmap->buckets.keys[b];
// 	hashmap->buckets.values[a]    = hashmap->buckets.values[b];

// 	hashmap->buckets.metas[a]     = tmp_meta;
// 	hashmap->buckets.distances[a] = tmp_distance;
// 	hashmap->buckets.keys[a]      = tmp_key;
// 	hashmap->buckets.values[a]    = tmp_value;

// 	/**
// 	 * todo
// 	 *   - [ ] Check if this compile to something different
// 	 */
// 	// tmp_meta                  = hashmap->buckets.metas[a];
// 	// hashmap->buckets.metas[a] = hashmap->buckets.metas[b];
// 	// hashmap->buckets.metas[a] = tmp_meta;

// 	// tmp_distance                  = hashmap->buckets.distances[a];
// 	// hashmap->buckets.distances[a] = hashmap->buckets.distances[b];
// 	// hashmap->buckets.distances[a] = tmp_distance;

// 	// tmp_key                  = hashmap->buckets.keys[a];
// 	// hashmap->buckets.keys[a] = hashmap->buckets.keys[b];

// 	// hashmap->buckets.keys[a]   = tmp_key;
// 	// tmp_value                  = hashmap->buckets.values[a];
// 	// hashmap->buckets.values[a] = hashmap->buckets.values[b];
// 	// hashmap->buckets.values[a] = tmp_value;

// 	return;
// }
//----------------------------------------------------------------- Function ---
/**
 * Check if given entry in given hashmap is empty
 *   -> Truth value of predicate
 */
static inline int is_empty(const ctrl_byte control)
{
	return (control == CTRL_EMPTY);
}
static inline int hmap_is_empty(struct hmap *hashmap, size_t entry)
{
	return is_empty(hashmap->buckets.metas[entry]);
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
// size_t put_hashmap(struct hmap *hashmap, const char *key, void *value)
// {
// 	assert(key != NULL);
// 	assert(value != NULL);

// 	size_t hash = hash_tab((unsigned char *)key, hashmap->xor_seed);
// 	hashmap->stats.hashes_tally_or |= hash;
// 	hashmap->stats.hashes_tally_and &= hash;

// 	// printf("hash->[%lu]\n", hash);
// 	// HashmapEntry *bucket = &hashmap->buckets[hash];
// 	/**
// 	 * todo
// 	 *   - [ ] Check if this compile to something different
// 	 */
// 	// unsigned char *meta;
// 	// int * distance;
// 	// char **key;
// 	// void **value;

// 	unsigned char entry_meta = hash_meta(hash);
// 	size_t entry_index       = hash_index(hash);

// 	__m128i filter = _mm_set1_epi8(entry_meta);
// 	int match_mask = _mm_movemask_epi8(_mm_cmpeq_epi8(filter), );

// 	/* Prepare temp entry*/
// 	struct hmap_buckets entry;
// 	entry.distance = 0;
// 	entry.key      = XMALLOC(strlen(key) + 1, "put_hashmap", "temp_entry.key");
// 	strcpy(entry.key, key);
// 	entry.value = value;

// 	for (; entry.distance < hashmap->probe_limit; entry.distance++, bucket++) {
// 		/* Empty bucket */
// 		if (bucket->key == NULL) {
// 			bucket->key      = entry.key;
// 			bucket->value    = entry.value;
// 			bucket->distance = entry.distance;
// 			// check_hashmap(hashmap);
// 			hashmap->count++;

// 			/* Solved collision */
// 			if (entry.distance > 0) {
// 				hashmap->stats.collision_count++;
// 			}
// 			hashmap->stats.put_count++;
// 			return bucket;
// 		}
// 		else {
// 			/* Key exist */
// 			// todo
// 			//   - [ ] Probe for existing key
// 			//   - [ ] Make sure comparison is necessary and cheap
// 			if (strcmp(entry.key, bucket->key) == 0) {
// 				XFREE(entry.key, "duplicate key");
// 				hashmap->destructor(
// 				    bucket->value); //, "bucket->value", "destructor");
// 				bucket->value = entry.value;
// 				hashmap->stats.put_count++;
// 				return bucket;
// 			}

// 			// /* Rich bucket */
// 			// if (entry.distance > bucket->distance) {
// 			// 	swap_entries(&entry, bucket);
// 			// 	hashmap->stats.swap_count++;
// 			// }
// 		}
// 	}

// 	/* Past probe limit, trigger resize */
// 	XFREE(entry.key, "past probe limit");
// 	hashmap->stats.putfail_count++;
// 	return NULL;
// }

//----------------------------------------------------------------- Function ---
/**
 * Locate given key in given hmap
 *   -> entry index
 */
#ifdef DEBUG_HMAP
void print_m128i(const __m128i value)
{
	printf("%016llx%016llx\n", value[1], value[0]);
}

void print_m128i_hexu8(const __m128i value)
{
	// _Alignas(uint16_t) uint16_t vec[8];
	uint8_t vec[16];
	_mm_store_si128((__m128i *)vec, value);
	printf(
	    "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x "
	    "%02x %02x\n",
	    vec[0],
	    vec[1],
	    vec[2],
	    vec[3],
	    vec[4],
	    vec[5],
	    vec[6],
	    vec[7],
	    vec[8],
	    vec[9],
	    vec[10],
	    vec[11],
	    vec[12],
	    vec[13],
	    vec[14],
	    vec[15]);
}
#endif /* DEBUG_HMAP */

size_t hmap_find(const struct hmap *hashmap, const char *key)
{
	size_t hash    = hash_tab((unsigned char *)key, hashmap->xor_seed);
	size_t index   = hash_index(hash);
	ctrl_byte meta = hash_meta(hash);
	// size_t entry;
	int match_mask;

	//----------------------------------------------------- setup filter
	const __m128i filter = _mm_set1_epi8(meta);
#ifdef DEBUG_HMAP
	puts("filter :");
	print_m128i_hexu8(filter);
#endif /* DEBUG_HMAP */

	//---------------------------------------------------- filter groups
	for (size_t i = 0;; i += 16) {
		__m128i group =
		    _mm_loadu_si128((__m128i *)(hashmap->buckets.metas + index));
#ifdef DEBUG_HMAP
		print_m128i_hexu8(group);
#endif /* DEBUG_HMAP */
		__m128i match = _mm_cmpeq_epi8(filter, group);
		match_mask    = _mm_movemask_epi8(match);

		//-------------------------------- use bitmask to access matches
		while (match_mask != 0) {
			// int least_significant_set_bit_only = match_mask & -match_mask;
			size_t offset = __builtin_ctzl(match_mask);
			if ((strcmp(hashmap->buckets.keys[index + offset], key)) == 0) {
#ifdef DEBUG_HMAP
				printf("\t%x -> %d match @ [%lu]\n",
				       meta,
				       _mm_popcnt_u32((unsigned int)match_mask),
				       offset);
#endif /* DEBUG_HMAP */
				return index + offset;
			}
			match_mask ^= match_mask & -match_mask;
		}
	}
	/**
	 * todo
	 *   - [ ] fix silly return value
	 */
	return -1;
}

//----------------------------------------------------------------- Function ---
/**
 * Locate given value for given key in given hmap
 *   -> pointer to value in hmap
 */
// void *hmap_get(const struct hmap *hashmap, const char *key) {}

//----------------------------------------------------------------- Function ---
/**
 * Free given HashmapEntry
 *   -> nothing
 *
 * note
 *   Require user to allocate for given value to be inserted in the hashmap
 *   Require user to provide destructor for given value
 *   Free value with given destructor
 */
static inline void destroy_entry(struct hmap *hashmap, size_t entry)
{
	// Use XFREE macro instead of destructor while hmap is a WIP
	XFREE(hashmap->buckets.keys[entry], hashmap->buckets.keys[entry]);
	// destructor(hashmap->buckets.keys[entry]);
	XFREE(hashmap->buckets.values[entry], "destroy_entry");
	return;
}
//----------------------------------------------------------------- Function ---
/**
 * Locate HasmapEntry for given key in given Hashmap
 * If found
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
 *
 * todo
 *   - [ ] Consider a clear_hashmap func : destroy entries only
 *   - [ ] Consider a reset_hashmap func : destroy entries, reset hashmap to
 *         given new size
 */
void hmap_delete_hashmap(struct hmap *hashmap)
{
	size_t actual_capacity = hashmap->capacity + hashmap->probe_limit;

	for (size_t entry = 0; entry < actual_capacity; entry++) {
		if (hashmap->buckets.keys[entry] == NULL) {
			continue;
		}
		// printf("%s\n", hashmap->buckets[i].key);
		destroy_entry(hashmap, entry);
#ifdef DEBUG_HMAP
		hashmap->stats.del_count++;
#endif /* DEBUG_HMAP */
	}
	// stats_hashmap(hashmap);
	XFREE(hashmap->buckets.metas, "delete_hashmap hashmap->buckets.metas");
	XFREE(hashmap->buckets.distances,
	      "delete_hashmap hashmap->buckets.distances");
	XFREE(hashmap->buckets.keys, "delete_hashmap hashmap->buckets.keys");
	XFREE(hashmap->buckets.values, "delete_hashmap hashmap->buckets.value");
	XFREE(hashmap, "delete_hashmap");
}

//----------------------------------------------------------------- Function ---
/**
 * Create a new xor_seed for given capacity
 *   -> pointer to new xor_seed
 *
 * todo
 *   - [ ] Consider splitting xor_seed generation to its own function
 */

//----------------------------------------------------------------- Function ---
/**
 * Create a new hmap of capacity equal to 2^(given n),  with given seed
 *
 *   Enforce capacity >= 256
 *   Init a dummy xor_seed with hash_depth set to yield capacity + 7 ctrl bits
 *   Init a dummy hmap_init
 * 	 Copy dummy xor_seed in dummy HashmapInit
 *   Allocate a Hashmap
 *   Copy dummy HashmapInit to Hashmap
 *   Allocate Hashmap buckets
 *   Init Hashmap buckets to NULL
 *   Init insertion list
 *     -> pointer to new hmap
 *
 * todo
 *   - [ ] Replace parameter n with requested_capacity
 *     + [ ] Compute next 2^(n) - 1 from requested_capacity
 *     + [ ] Store n
 *   - [ ] Make sure hashmap_init cannot blow up the stack on HUGE sizes
 *     + [ ] Handle such cases if any with branching to an alloc
 *     + [ ] Consider dropping the whole hashmap_init step and the const
 *           qualifier on xor_seed_init as workaround
 *   - [ ] Consider one big alloc of
 *         sizeof(metas + distances +  keys + values ) * actual_capacity
 *     + [ ] Consider setting pointers inside the struct accordingly
 *           after the one big alloc
 */
struct hmap *hmap_new(const unsigned int n,
                      const struct hmap_allocator *const allocator)
{
	//--------------------------------------------------- allocator init
	struct hmap_allocator allocator_init;

	if (allocator == NULL) {
		allocator_init.malloc = malloc;
		allocator_init.free   = free;
	}
	else {
		allocator_init = *allocator;
	}
	//---------------------------------------------- advertised capacity
	/* current tab_hash implementation requires a mininum of 256 slots */
	// const size_t capacity   = (n < 8) ? (1u << 8) : (1u << n);
	/* the 7 extra bits are required anyway for ctrl_byte */
	const size_t capacity   = 1u << n;
	const size_t hash_depth = capacity << 7;
	//---------------------------------------------------- xor_seed init
	size_t xor_seed_init[256];
	srand(__rdtsc());
	for (size_t i = 0; i < 256; i++) {
		// % capacity is okay-ish as long as its a power of 2
		// Expect skewed distribution otherwise
		xor_seed_init[i] = rand() % (hash_depth);
	}

	//-------------------------------------------------------- hmap init
	const size_t actual_capacity = capacity + PROBE_LIMIT;

#ifdef DEBUG_HMAP
	struct hmap_stats stats_init  = {0, SIZE_MAX, 0, 0, 0, 0, 0, 0, 0};
	struct hmap_init hashmap_init = {
	    {0}, {NULL}, allocator_init, n, PROBE_LIMIT, capacity, 0, stats_init};
#else
	struct hmap_init hashmap_init = {
	    {0}, {NULL}, allocator_init, n, PROBE_LIMIT, capacity, 0};
#endif /* DEBUG_HMAP */

	memcpy(hashmap_init.xor_seed, xor_seed_init, sizeof(xor_seed_init));

	struct hmap *new_hashmap =
	    XMALLOC(sizeof(struct hmap), "new_hashmap", "new_hashmap");
	memcpy(new_hashmap, &hashmap_init, sizeof(struct hmap));
	if (new_hashmap == NULL) {
		goto err_free_hashmap;
	}

	new_hashmap->buckets.metas =
	    XMALLOC(sizeof(*(new_hashmap->buckets.metas)) * actual_capacity,
	            "new_hashmap",
	            "new_hashmap->buckets.metas");
	if (new_hashmap->buckets.metas == NULL) {
		goto err_free_buckets;
	}
	new_hashmap->buckets.distances =
	    XMALLOC(sizeof(*(new_hashmap->buckets.distances)) * actual_capacity,
	            "new_hashmap",
	            "new_hashmap->buckets.distances");
	if (new_hashmap->buckets.metas == NULL) {
		goto err_free_buckets;
	}
	new_hashmap->buckets.keys =
	    XMALLOC(sizeof(*(new_hashmap->buckets.keys)) * actual_capacity,
	            "new_hashmap",
	            "new_hashmap->buckets.keys");
	if (new_hashmap->buckets.metas == NULL) {
		goto err_free_buckets;
	}
	new_hashmap->buckets.values =
	    XMALLOC(sizeof(*(new_hashmap->buckets.values)) * actual_capacity,
	            "new_hashmap",
	            "new_hashmap->buckets.values");
	if (new_hashmap->buckets.metas == NULL) {
		goto err_free_buckets;
	}

	/* The value is passed as an int, but the function fills the block of
	 * memory using the unsigned char conversion of this value */
	memset(new_hashmap->buckets.metas,
	       CTRL_EMPTY,
	       sizeof(*(new_hashmap->buckets.metas)) * actual_capacity);
	/* Is this enough to be able to check if ptr == NULL ? */
	memset(new_hashmap->buckets.distances,
	       0,
	       sizeof(*(new_hashmap->buckets.distances)) * actual_capacity);
	memset(new_hashmap->buckets.keys,
	       0,
	       sizeof(*(new_hashmap->buckets.keys)) * actual_capacity);
	memset(new_hashmap->buckets.values,
	       0,
	       sizeof(*(new_hashmap->buckets.values)) * actual_capacity);

	return new_hashmap;

/* free(NULL) is ok, correct ? */
err_free_buckets:
	XFREE(new_hashmap->buckets.metas, "new_hashmap->buckets.metas");
	XFREE(new_hashmap->buckets.distances, "new_hashmap->buckets.distances");
	XFREE(new_hashmap->buckets.keys, "new_hashmap->buckets.keys");
	XFREE(new_hashmap->buckets.values, "new_hashmap->buckets.values");
err_free_hashmap:
	XFREE(new_hashmap, "new_hashmap");
	return NULL;
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
void print_xor_seed(const struct hmap *hashmap)
{
	puts("xor_seed            :");
	for (size_t i = 0; i < 256; i++) {
		printf("%0*lx ",
		       (int)ceil(((hashmap->n + 7) / 4.0)),
		       hashmap->xor_seed[i]);
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
#ifdef DEBUG_HMAP
void stats_hashmap(const struct hmap *hashmap)
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
	printf("hashmap->stats.hashes_ctrl_collision_count  : %lu\n",
	       hashmap->stats.collision_count);

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

	return;
}
#endif /* DEBUG_HMAP */
//----------------------------------------------------------------- Function ---
/**
 * Dump given Hashmap content
 *   -> nothing
 *
 * todo
 *   - [ ] Implement list iterator
 *     + [ ] See http://rosettacode.org/wiki/Doubly-linked_list/Traversal#C
 */
void dump_hashmap(const struct hmap *hashmap)
{
	size_t empty_bucket = 0;

	for (size_t i = 0; i < (hashmap->capacity + hashmap->probe_limit); i++) {
		if (hashmap->buckets.keys[i] == NULL) {
			empty_bucket++;
			printf("\x1b[100mhashmap->\x1b[30mbucket[%lu]>> EMPTY <<\x1b[0m\n",
			       i);
			continue;
		}

		printf("\x1b[10%dmhashmap->\x1b[30mbucket[%lu]\x1b[0m>>\n", 4, i);

		// printf("\t %016lx : %s\n",
		printf("\t        %lu : %s\n",
		       hash_tab((unsigned char *)hashmap->buckets.keys[i],
		                hashmap->xor_seed),
		       hashmap->buckets.keys[i]);
	}

	printf("empty_buckets       : %lu \t-> %f%%\n",
	       empty_bucket,
	       (double)empty_bucket / (double)hashmap->capacity * 100);

#ifdef DEBUG_HMAP
	stats_hashmap(hashmap);
#endif /* DEBUG_HMAP */

	print_xor_seed(hashmap);
}

//--------------------------------------------------------------------- MAIN ---
int main(void)
{
	// uint32_t seed = 31;
	// todo
	//   - [x] Confirm tab_hash should NOT work for n < 8 as it is
	size_t n = 8;
	size_t hash;
	// Hashmap *hashmap = new_hashmap(n, seed, hash_fimur_reduce);

	fputs(FG_BRIGHT_BLUE REVERSE
	      "Table size is 2^n. Enter n ( default n=8 ) " FG_BRIGHT_BLUE
	      " ? " RESET,
	      stdout);
	scanf("%lu", &n);

	struct hmap *hashmap = hmap_new(n, NULL);
	//----------------------------------------------------------- mock entry
	hash           = hash_tab((unsigned char *)"mock", hashmap->xor_seed);
	size_t index   = hash_index(hash);
	ctrl_byte meta = hash_meta(hash);
	hashmap->buckets.metas[index]  = meta;
	hashmap->buckets.keys[index]   = strdup("mock");
	hashmap->buckets.values[index] = strdup("mock value");
	hashmap->count++;

	hashmap->buckets.metas[index + 3]  = meta;
	hashmap->buckets.keys[index + 3]   = strdup("mock");
	hashmap->buckets.values[index + 3] = strdup("mock value");
	hashmap->count++;

	hmap_find(hashmap, "mock");

//-------------------------------------------------------------------- setup
#define KEYPOOL_SIZE 32
	// char random_keys[KEYPOOL_SIZE] = {'\0'};
	// size_t test_count              = (1 << (n - 2)) - 1; // 1 << (n - 1);
	char key[256];
	// char *dummy_value = NULL;

	// srand(__rdtsc());

	// for (size_t k = 0; k < test_count; k++) {
	// 	for (size_t i = 0; i < KEYPOOL_SIZE - 1; i++) {
	// 		random_keys[i] = (char)(rand() % 95 + 0x20); // % 26 + 0x61);
	// 		// putchar(random_keys[i]);
	// 	}
	// 	// putchar('\n');

	// 	dummy_value = strdup(&random_keys[rand() % (KEYPOOL_SIZE - 1)]);
	// 	// printf(
	// 	//     "[%lu]key   : %s\n"
	// 	//     "[%lu]value : %s\n",
	// 	//     k,
	// 	//     &random_keys[rand() % (KEYPOOL_SIZE - 1)],
	// 	//     k,
	// 	//     dummy_value);

	// 	if (put_hashmap(hashmap,
	// 	                &random_keys[rand() % (KEYPOOL_SIZE - 1)],
	// 	                dummy_value) == NULL) {
	// 		XFREE(dummy_value, "main : setup");
	// 	}
	// }

	//----------------------------------------------------------- input loop
	for (;;) {
		fputs("\x1b[102m > \x1b[0m", stdout);
		scanf("%s", key);
		// fgets(key, 255, stdin);

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
#ifdef DEBUG_HMAP
		if ((strcmp(key, "stats")) == 0) {
			stats_hashmap(hashmap);
			continue;
		}
#endif /* DEBUG_HMAP */

		if ((strcmp(key, "add")) == 0) {
			printf("todo\n");
			continue;
		}

		hash = hash_tab((unsigned char *)key, hashmap->xor_seed);
		printf(
		    "key        : %s\n"
		    "key        : %s\n"
		    "hash       : %lx\n"
		    "hash_index : %lx\n"
		    "hash_meta  : %x\n",
		    key,
		    (unsigned char *)key,
		    hash,
		    hash_index(hash),
		    hash_meta(hash));

		//-------------------------------------------- lookup prototype
		// const size_t index =
		//     hash_tab((unsigned char *)key, hashmap->xor_seed);
		// dummy_value = strdup(key);
		// if (put_hashmap(hashmap, key, dummy_value) == NULL) {
		// 	XFREE(dummy_value, "main : setup");
		// }
	}

	//-------------------------------------------------------------- cleanup
	hmap_delete_hashmap(hashmap);
	return 0;
}

/**
 * todo
 *   - [ ] Consider 'fast range' reduce
 *     + [ ] See :
 * https://lemire.me/blog/2016/06/27/a-fast-alternative-to-the-modulo-reduction/
 */
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