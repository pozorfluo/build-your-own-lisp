//------------------------------------------------------------------ SUMMARY ---
/**
 * Based on the work of the Abseil Team and publications of Daniel Lemire,
 * Peter Kankowski, Malte Skarupke, Maurice Herlihy, Nir Shavit, Moran Tzafrir,
 * Emmanuel Goossaert.
 *
 * Implement a hash map tuned for lispy / LispEnv
 */

#include <errno.h>
#include <inttypes.h> /* strtoumax */
#include <stddef.h>   /* size_t */
#include <stdint.h>   /* uint32_t, uint64_t */
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* memcpy */

#include "ansi_esc.h"

#include "debug_xmalloc.h"
//------------------------------------------------------------ CONFIGURATION ---
#include "configuration.h"
//------------------------------------------------------------ MAGIC NUMBERS ---
#define HMAP_NOT_FOUND SIZE_MAX

#ifdef SIMD_PROBE
#ifdef __AVX__
/**
 *   _mm256_set_epi8
 *   _mm256_cmpeq_epi8
 *   _mm256_cmpgt_epi8
 *   _mm256_movemask_epi8
 *   _mm256_store_si256
 *   _mm256_loadu_si256
 *   _mm256_lddqu_si256
 */
/**
 * __rdtsc
 * __builtin_ctz, __builtin_ctzl, __builtin_ctzll
 * _bit_scan_forward, _BitScanForward, _BitScanForward64
 */
#ifdef _MSC_VER
#include <intrin.h>
#else
#include <immintrin.h>
#include <x86intrin.h>
#endif /* _MSC_VER */
#define PROBE_LENGTH 32
#else
/**
 * require at least __SSE2__
 *   _mm_set1_epi8
 *   _mm_cmpeq_epi8
 *   _mm_cmpgt_epi8
 *   _mm_movemask_epi8
 *   _mm_cmpeq_epi8
 *   _mm_store_si128
 *   _mm_loadu_si128
 *
 * require at least __SSE3__
 *   _mm_lddqu_si128
 */
#ifdef _MSC_VER
#include <intrin.h>
#else
#include <immintrin.h>
#include <x86intrin.h>
#endif /* _MSC_VER */
#define PROBE_LENGTH 16
#endif /* __AVX__ */
#else
#define PROBE_LENGTH 32
#endif /* SIMD_PROBE */

//------------------------------------------------------------------- MACROS ---

//------------------------------------------------------------- DECLARATIONS ---
typedef signed char meta_byte;

enum meta_ctrl {
	META_EMPTY    = -128, /* 0b10000000 */
	META_OCCUPIED = 0,    /* 0b0xxxxxxx */
	/* Think of META_OCCUPIED as anything like 0b0xxxxxxx */
	/* aka 0 <= META_OCCUPIED < 128 */
};

struct hmap_entry {
	size_t key;
	size_t value;
};

struct hmap_buckets {
	meta_byte *restrict metas;     /* 1 byte per bucket,*/
	meta_byte *restrict distances; /* 1 byte per bucket,*/
	size_t *restrict entries;
};

/**
 * Check packing https://godbolt.org/z/dvdKqM
 */
struct hmap {
	struct hmap_buckets buckets;
	struct hmap_entry *store;
	size_t top;
	size_t hash_shift; /* shift amount necessary for desired hash depth */
	size_t capacity;   /* actual capacity */
	size_t count;      /* occupied entries count */
};

//----------------------------------------------------- FORWARD DECLARATIONS ---
static inline size_t reduce_fibo(const size_t hash, const size_t shift)
    __attribute__((const, always_inline));

static inline size_t hash_index(const size_t hash)
    __attribute__((const, always_inline));

static inline meta_byte hash_meta(const size_t hash)
    __attribute__((const, always_inline));

#ifdef __AVX__
static inline uint32_t probe_pattern(const meta_byte pattern,
                                     const meta_byte *const entry)
    __attribute__((pure, always_inline));
#else
static inline uint16_t probe_pattern(const meta_byte pattern,
                                     const meta_byte *const entry)
    __attribute__((pure, always_inline));
#endif /* __AVX__ */

static inline void destroy_entry(struct hmap *const hashmap, const size_t entry)
    __attribute__((always_inline));

size_t hmap_find(const struct hmap *const hashmap, const size_t key)
    __attribute__((pure));

static inline size_t hmap_find_or_empty(const struct hmap *const hashmap,
                                        const size_t key,
                                        size_t index,
                                        const meta_byte meta)
    __attribute__((pure, always_inline));

void dump_hashmap(const struct hmap *const hashmap);

//----------------------------------------------------------------- Function ---
static inline size_t hash_index(const size_t hash) { return hash >> 7; }

static inline meta_byte hash_meta(const size_t hash) { return hash & 0x7F; }

//----------------------------------------------------------------- Function ---
/**
 * Reduce a given hash to a packed hmap index and meta_byte
 *
 * also can be used as :
 *
 * Compute a hash, multiplicative style with a fibonacci derived constant, for a
 * given numeric key
 *
 *   -> packed hmap index and meta_byte
 *
 * note
 *   This assumes that the max size possibly requested for hmap is 2^57 or
 * 2^( 64 - 7 bits of "extra hash" required for meta_byte )
 *
 *   i.e., 144,115,188,075,855,872 max advertised capacity
 */
static inline size_t reduce_fibo(const size_t hash, const size_t shift)
{
	const size_t xor_hash = hash ^ (hash >> shift);
	return (11400714819323198485llu * xor_hash) >> shift;
}
//----------------------------------------------------------------- Function ---
/**
 * Check if bucket at given index in given hashmap is empty
 *   -> Truth value of predicate
 */
static inline int is_empty(const meta_byte meta)
{
	return (meta == META_EMPTY);
}
static inline int is_bucket_empty(const struct hmap *const hashmap,
                                  const size_t index)
{
	return is_empty(hashmap->buckets.metas[index]);
}

//----------------------------------------------------------------- Function ---
/**
 * Check if bucket at given index in given hashmap is occupied
 *   -> Truth value of predicate
 */
static inline int is_occupied(const meta_byte meta)
{
	return (meta >= META_OCCUPIED);
}

static inline int is_bucket_occupied(const struct hmap *const hashmap,
                                     const size_t index)
{
	return is_occupied(hashmap->buckets.metas[index]);
}

//----------------------------------------------------------------- Function ---
/**
 * Probe metadata chunk of 16 bytes for given meta_byte pattern starting at
 * given metadata entry
 *   -> Matches bitmask
 */
#ifdef __AVX__
static inline uint32_t probe_pattern(const meta_byte pattern,
                                     const meta_byte *const entry_meta)
{
	/* setup filter */
	const __m256i filter = _mm256_set1_epi8(pattern);

	/* filter chunks */
	const __m256i chunk = _mm256_loadu_si256((__m256i *)(entry_meta));
	const __m256i match = _mm256_cmpeq_epi8(filter, chunk);

	return _mm256_movemask_epi8(match);
}
#else
static inline uint16_t probe_pattern(const meta_byte pattern,
                                     const meta_byte *const entry_meta)
{
	/* setup filter */
	const __m128i filter = _mm_set1_epi8(pattern);

	/* filter chunks */
	const __m128i chunk = _mm_loadu_si128((__m128i *)(entry_meta));
	const __m128i match = _mm_cmpeq_epi8(filter, chunk);

	return _mm_movemask_epi8(match);
}
#endif /* __AVX__ */

//----------------------------------------------------------------- Function ---
/**
 * Locate given key in given hmap
 * If given key exists
 *   -> entry index
 * Else
 *   -> out of bound value ( > capacity)
 *
 * note
 *   Using an out of bound value as an error condition means that 1 value must
 *   be reserved for it. Implicitely it says that the maximum allowed
 *   capacity is SIZE_MAX - 1
 *
 * todo
 *   - [ ] Investigate ways to probe for Match or META_EMPTY at once
 *   - [X] Consider using SIZE_MAX directly as an error
 *     + [ ] Add exists() inlinable function
 */
size_t hmap_find(const struct hmap *const hashmap, const size_t key)
{
	size_t hash    = reduce_fibo(key, hashmap->hash_shift);
	size_t index   = hash_index(hash);
	meta_byte meta = hash_meta(hash);

#ifdef __AVX__
	uint32_t match_mask;
#else
	uint16_t match_mask;
#endif /* __AVX__ */

	meta_byte *chunk;

	do {
		chunk      = hashmap->buckets.metas + index;
		match_mask = probe_pattern(meta, chunk);

		/* loop through set bit in bitmask to access matches */
		while (match_mask != 0) {
			const size_t offset = __builtin_ctzl(match_mask); // uint32_t
			size_t match        = index + offset;

			if (hashmap->store[(hashmap->buckets.entries[match])].key == key) {
				/* Found key ! */
				return match;
			}
			/* remove least significant set bit */
			match_mask ^= match_mask & (-match_mask);
		}
		/* no match in current chunk */

		/* if there is any empty slot in the chunk that was probed */
		/* no need to check the next */
		/* -> key does NOT exist */
		if (probe_pattern(META_EMPTY, chunk)) {
			break;
		}

		index += PROBE_LENGTH;
		/* THIS SHOULD HARDLY EVER BE REACHED FOR LOAD FACTOR <= 0.95 */
		/* chunk done */
	} while (index < hashmap->capacity);

	/* key not found */
	// return hashmap->capacity + 1;
	return HMAP_NOT_FOUND;
}
//----------------------------------------------------------------- Function ---
/**
 * Locate given key in given hmap starting at given index looking for given
 * meta_byte pattern
 *
 * If given key exists
 *   -> entry index
 * Else
 *   -> first empty slot
 *
 * todo
 *   - [ ] Investigate ways to probe for Match or META_EMPTY at once
 */
static inline size_t hmap_find_or_empty(const struct hmap *const hashmap,
                                        const size_t key,
                                        size_t index,
                                        const meta_byte meta)
{
#ifdef __AVX__
	uint32_t match_mask;
#else
	uint16_t match_mask;
#endif /* __AVX__ */

	meta_byte *chunk;

	do {
		chunk      = hashmap->buckets.metas + index;
		match_mask = probe_pattern(meta, chunk);

		/* loop through set bit in bitmask to access matches */
		while (match_mask != 0) {
			const size_t offset = __builtin_ctzl(match_mask); // uint32_t
			size_t match        = index + offset;

			if (hashmap->store[(hashmap->buckets.entries[match])].key == key) {
				/* Found key ! */
				return match;
			}
			/* clear least significant set bit */
			match_mask ^= match_mask & (-match_mask);
		}
		/* no match in current chunk */

		/* if there is any empty slot in the chunk that was probed */
		/* no need to check the next */
		if ((match_mask = probe_pattern(META_EMPTY, chunk))) {
			/* -> key does NOT exist, insert in first empty slot */
			return index + __builtin_ctzl(match_mask);
		}

		index += PROBE_LENGTH;
		/* THIS SHOULD HARDLY EVER BE REACHED FOR LOAD FACTOR <= 0.95 */
		/* chunk done */
	} while (index < hashmap->capacity);

	/* key not found, no empty slots left */
	// return hashmap->capacity + 1;
	return HMAP_NOT_FOUND;
}

//----------------------------------------------------------------- Function ---
/**
 * Locate given value for given key in given hmap
 * If given key exists
 *   -> pointer to value in hmap
 * Else
 *   //-> NULL
 *   -> 0
 */
size_t hmap_get(const struct hmap *const hm, const size_t key)
{
	const size_t entry = hmap_find(hm, key);
	// void *value        = NULL;
	size_t value = 0;

	if (entry != HMAP_NOT_FOUND) {
		value = hm->store[(hm->buckets.entries[entry])].value;
	}

	return value;
}

//----------------------------------------------------------------- Function ---
/**
 * Insert a new entry in given Hashmap for given key, value pair or update
 * an
 * existing value
 *
 * Find given key or first empty slot
 * If given key found
 *   Update it with given value
 *     -> updated entry index
 * Else If empty slot found
 *   Thierry La Fronde method : Slingshot the rich !
 *     Go backwards from candidate empty bucket to home
 *       If current entry distance is lower or equal than the entry upstream
 *         Slingshot it to candidate empty bucket !
 *         Make current entry the new candidate empty bucket
 *     Slingshot given entry to wherever candidate empty bucket is
 *     Increment hmap entry count
 *       -> new entry index
 *
 * note
 * 	 Find returns existing key or first empty bucket
 *   By definition there should be no other empty bucket between home
 *   and candidate bucket
 *
 *   Distance to home is stored on a single byte which means there
 *   can NOT be more than 256 slingshot jobs to resolve. This is
 *   probably overly generous already.
 *
 *   Do not allow a full table, resize before it happens
 *     -> hmap_put does not handle inserting on a full table
 *   Is limited to chain <= 256 due to distances meta_byte type
 *
 * todo
 *   - [ ] Add hmap fitness / resize trigger logic
 */
size_t hmap_put(struct hmap *const hm, const size_t key, const size_t value)
{
	/* Prepare temp entry */
	const size_t hash    = reduce_fibo(key, hm->hash_shift);
	const size_t home    = hash_index(hash);
	const meta_byte meta = hash_meta(hash);

	/* Find given key or first empty slot */
	size_t candidate = hmap_find_or_empty(hm, key, home, meta);

	//----------------------------------------------------- empty slot found
	if (is_empty(hm->buckets.metas[candidate])) {
		/* Thierry La Fronde method : Slingshot the rich ! */
		for (size_t bucket = candidate; bucket != home; bucket--) {
			hm->buckets.distances[candidate] = candidate - home;

			if (hm->buckets.distances[bucket] <=
			    hm->buckets.distances[bucket - 1]) {

				hm->buckets.distances[candidate] =
				    hm->buckets.distances[bucket] + candidate - bucket;

				hm->buckets.metas[candidate] = hm->buckets.metas[bucket];

				hm->buckets.entries[candidate] = hm->buckets.entries[bucket];
				candidate                      = bucket;
			}
		}

		hm->buckets.metas[candidate]     = meta;
		hm->buckets.distances[candidate] = candidate - home;
		hm->buckets.entries[candidate]   = hm->top;

		hm->store[hm->top].key   = key;
		hm->store[hm->top].value = value;
		hm->top++;
		hm->count++;
	}
	//------------------------------------------------------ given key found
	else {
		hm->store[(hm->buckets.entries[candidate])].value = value;
	}
	/* -> new or updated entry index */
	return candidate;
}

//----------------------------------------------------------------- Function ---
/**
 * Mark metadata associated to given entry as META_EMPTY
 * Update given hmap stats
 *   -> removed	entry index
 */
static inline void empty_entry(struct hmap *const hm, const size_t entry)
{
	hm->buckets.metas[entry] = META_EMPTY;
	hm->count--;
}
//----------------------------------------------------------------- Function ---
/**
 * Free given hmap_entry
 *
 * Rewind top
 * Find top store entry corresponding bucket entry
 * Move top store entry to destroyed store entry slot
 * Wipe top
 *   -> nothing
 *
 * note
 *   This should probably not be used with the store.
 *   Just mark the entry as empty.
 *   Free the store sequentially when deleting the hmap.
 *   Keep destroy_entry only to specifically wipe an entry from the store.
 */
static inline void destroy_entry(struct hmap *const hm, const size_t entry)
{
	/**

	 */
	const size_t store_slot = hm->buckets.entries[entry];
	hm->top--;

	if ((hm->top > 0) && (hm->top != store_slot)) {
		const size_t top_key = hm->store[hm->top].key;

		size_t top_bucket = hmap_find(hm, top_key);

		hm->buckets.entries[top_bucket] = hm->buckets.entries[entry];

		hm->store[(hm->buckets.entries[entry])].key =
		    hm->store[hm->top].key;
		hm->store[(hm->buckets.entries[entry])].value =
		    hm->store[hm->top].value;
	}
	return;
}

//----------------------------------------------------------------- Function ---
/**
 * Look for given key in given hmap
 *
 * If given key exists
 *   update the store
 * 	 probe for stop bucket
 *   slingshot entry backward, up to stop bucket
 *   mark entry right before stop bucket as empty
 *   -> removed	entry index
 * Else
 *   -> out of bound value ( > capacity )
 *
 */
size_t hmap_remove(struct hmap *const hm, const size_t key)
{
	const size_t entry = hmap_find(hm, key);

	/* Given key exists */
	if (entry != HMAP_NOT_FOUND) {
		/* update store */
		destroy_entry(hm, entry);

/* probe for stop bucket */
#ifdef __AVX__
		uint32_t match_mask;
#else
		uint16_t match_mask;
#endif /* __AVX__ */

		/**
		 * todo
		 *   - [ ] Assess if probing at entry + 1 yelds a cache miss when
		 *         read/writing to entry
		 *     + [ ] Do not bother ! probing at (advertised)capacity + 1 will
		 *            try to read 1 bucket out of bound !!
		 *     + [ ] Be careful though, you are going to have to ignore entry
		 *           distance because it may be @home
		 */
		size_t stop_bucket = entry + 1;
		match_mask = probe_pattern(0, hm->buckets.distances + stop_bucket);

		/* is there at least 1 slingshot job ? */
		if (match_mask != 0) {
			/* slingshot entry backward, up to stop bucket */
			stop_bucket += _bit_scan_forward(match_mask);

			/* process buckets.distances while it is hot */
			for (size_t bucket = entry; bucket < stop_bucket; bucket++) {
				hm->buckets.distances[bucket] =
				    hm->buckets.distances[bucket + 1] - 1;
			}
			/* mark entry distance in last shifted bucket as @home or empty */
			hm->buckets.distances[stop_bucket - 1] = 0;

			/* process buckets.entries */
			for (size_t bucket = entry; bucket < stop_bucket; bucket++) {
				hm->buckets.entries[bucket] =
				    hm->buckets.entries[bucket + 1];
			}

			/* process buckets.metas */
			for (size_t bucket = entry; bucket < stop_bucket; bucket++) {
				hm->buckets.metas[bucket] =
				    hm->buckets.metas[bucket + 1];
			}
		}
		/* mark entry in last shifted bucket as empty */
		hm->buckets.metas[stop_bucket - 1] = META_EMPTY;
	}

	return entry;
}

//----------------------------------------------------------------- Function ---
/**
 * Free given Hashmap
 *   -> nothing
 */
void hmap_delete_hashmap(struct hmap *const hm)
{
	/* hashmap->buckets.metas is the head of the single alloc pool */
	XFREE(hm->buckets.metas, "hmap_delete_hashmap");
	XFREE(hm, "delete_hashmap");
}

//----------------------------------------------------------------- Function ---
/**
 * Create a new hmap of capacity equal to 2^(given n) + PROBE_LENGTH
 *
 *   Enforce n >= 1
 *   Allocate a Hashmap
 *     If allocation fails,  clean up and abort
 *     -> NULL
 *   Allocate and split a memory pool for metadata, the map, the store
 *     If any allocation fails, clean up and abort
 *     -> NULL
 *   Init Hashmap buckets
 *     -> pointer to new hmap
 *
 * note
 *   hashmap->n is the shift amount necessary to achieve the desired hash depth
 *   and is used with reduce function
 *
 * todo
 *   - [ ] Replace parameter n with requested_capacity
 *     + [ ] Compute next 2^(n) - 1 from requested_capacity
 *   - [ ] Clear confusion around n / shift amount and rename n
 */
struct hmap *hmap_new(const size_t n)
{
	//---------------------------------------------- advertised capacity
	/* tab_hash requires a mininum of 8 bits of hash space */
	/* including the 7 bits required for meta_byte */
	size_t capacity = (n < 1) ? (1u << 1) : (1u << n);
	// const size_t hash_depth = capacity << 7;

	//-------------------------------------------------------- hmap init
	/* actual capacity */
	capacity += PROBE_LENGTH;

	/**
	 * shift amount necessary for desired hash depth including the 7 bits
	 * required for meta_byte with reduce function
	 *
	 * note
	 *   this assumes __WORDSIZE == 64
	 */
	const size_t hash_shift = 64 - 7 - n;

	struct hmap hashmap_init = {{NULL}, NULL, 0, hash_shift, capacity, 0};

	struct hmap *const new_hashmap =
	    XMALLOC(sizeof(struct hmap), "new_hashmap", "new_hashmap");
	memcpy(new_hashmap, &hashmap_init, sizeof(struct hmap));

	if (new_hashmap == NULL) {
		goto err_free_hashmap;
	}

	//----------------------------------------------------- buckets init
	const size_t metas_size = sizeof(*(new_hashmap->buckets.metas)) * capacity;
	const size_t distances_size =
	    sizeof(*(new_hashmap->buckets.distances)) * capacity;
	const size_t entries_size =
	    sizeof(*(new_hashmap->buckets.entries)) * capacity;
	const size_t store_size = sizeof(*(new_hashmap->store)) * capacity;

	char *const pool =
	    XMALLOC(metas_size + distances_size + entries_size + store_size,
	            "new_hashmap",
	            "pool");

	if (pool == NULL) {
		goto err_free_pool;
	}

	new_hashmap->buckets.metas     = (meta_byte *)pool;
	new_hashmap->buckets.distances = (meta_byte *)(pool + metas_size);
	new_hashmap->buckets.entries =
	    (size_t *)(pool + metas_size + distances_size);
	new_hashmap->store = (struct hmap_entry *)(pool + metas_size +
	                                           distances_size + entries_size);

	/* The value is passed as an int, but the function fills the block of
	 * memory using the unsigned char conversion of this value */
	memset(new_hashmap->buckets.metas, META_EMPTY, metas_size);
	/**
	 * Because distances are initialized to 0
	 * and set to 0 when removing an entry
	 * Probing distances for 0 yields "stop buckets"
	 * aka @home entry or empty bucket
	 */
	memset(new_hashmap->buckets.distances, 0, distances_size);
	/* Is this enough to be able to check if ptr == NULL ? */
	memset(new_hashmap->buckets.entries, 0, entries_size);

	//------------------------------------------------------- store init
	return new_hashmap;

/* free(NULL) is ok, correct ? */
err_free_pool:
	XFREE(pool, "hmap_new err_free_arrays");
err_free_hashmap:
	XFREE(new_hashmap, "new_hashmap");
	return NULL;
}

//----------------------------------------------------------------- Function ---
/**
 * Dump given Hashmap content
 *   -> nothing
 */
void dump_hashmap(const struct hmap *const hm)
{
	size_t empty_bucket = 0;
	int max_distance    = 0;

	for (size_t i = 0; i < hm->capacity; i++) {
		if (hm->buckets.metas[i] == META_EMPTY) {
			empty_bucket++;
			printf("\x1b[100mhashmap->\x1b[30mbucket[%lu]>> EMPTY <<\x1b[0m\n",
			       i);
			continue;
		}

		/* Color code distance from home using ANSI esc code values */
		int colour;

		switch (hm->buckets.distances[i]) {
		case 0:
			colour = 4; // BLUE
			break;
		case 1:
			colour = 6; // CYAN
			break;
		case 2:
			colour = 2; // GREEN
			break;
		case 3:
			colour = 3; // YELLOW
			break;
		case 4:
			colour = 1; // RED
			break;
		case 5:
			colour = 5; // MAGENTA
			break;
		default:
			colour = 7; // WHITE
			break;
		}
		max_distance = (hm->buckets.distances[i] > max_distance)
		                   ? hm->buckets.distances[i]
		                   : max_distance;
		printf("\x1b[10%dmhashmap->\x1b[30mbucket[%lu]\x1b[0m>>", colour, i);

		printf(" %lu[%d] : %lu | %lu\n",
		       hash_index(reduce_fibo(
		           hm->store[(hm->buckets.entries[i])].key,
		           hm->hash_shift)),
		       hm->buckets.distances[i],
		       hm->store[(hm->buckets.entries[i])].key,
		       hm->store[(hm->buckets.entries[i])].value);
	}

	printf("empty_buckets       : %lu \t-> %f%%\n",
	       empty_bucket,
	       (double)empty_bucket / (double)(hm->capacity - PROBE_LENGTH) *
	           100);
	printf("max_distance        : %d\n", max_distance);
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
		sum_key += hm->store[(hm->buckets.entries[i])].key;
		sum_value += hm->store[(hm->buckets.entries[i])].value;
	}

	printf(FG_BLUE REVERSE
	       "sum_key             : %lu\n"
	       "sum_value           : %lu\n" RESET,
	       sum_key,
	       sum_value);
}

void sum_store(const struct hmap *const hm)
{
	size_t sum_key   = 0;
	size_t sum_value = 0;

	/* first rewind 1 entry from the top */
	size_t top = hm->top;
	while (top) {
		top--;
		sum_key += hm->store[top].key;
		sum_value += hm->store[top].value;
	}

	printf(FG_BLUE REVERSE
	       "sum_key             : %lu\n"
	       "sum_value           : %lu\n" RESET,
	       sum_key,
	       sum_value);
}

//--------------------------------------------------------------------- MAIN ---
int main(void)
{
	puts(WELCOME_MESSAGE);

#ifdef __AVX__
	puts("__AVX__ 1");
	printf("PROBE_LENGTH %d\n", PROBE_LENGTH);
#endif /* __AVX__ */

	printf("__WORDSIZE %d\n", __WORDSIZE);
	printf("RAND_MAX  %d\n", RAND_MAX);

	// uint32_t seed = 31;
	size_t n = 8;
	float load_factor;

	fputs(FG_BRIGHT_BLUE REVERSE
	      "Table size is 2^n. Enter n ( default n = 8 ) ? " RESET,
	      stdout);
	scanf("%lu", &n);

	struct hmap *const hashmap = hmap_new(n);

	fputs(FG_BLUE REVERSE "Enter desired load factor ? " RESET, stdout);
	scanf("%f", &load_factor);

	//---------------------------------------------------------------- setup
	SETUP_BENCH(repl);
	size_t load_count = (1 << n) * load_factor;
	printf("load_factor = %f\n", load_factor);
	printf("load_count  = %lu\n", load_count);
	size_t capacity  = hashmap->capacity;
	size_t sum_value = 0;
	char key[256];

	printf(FG_BRIGHT_YELLOW REVERSE "Filling hashmap with %lu entries\n" RESET,
	       load_count);

	printf(FG_BRIGHT_YELLOW REVERSE "hmap->top : %lu\n" RESET, hashmap->top);

	START_BENCH(repl);
	for (size_t k = 0; k < load_count; k++) {
		hmap_put(hashmap, k, k);
	}

	printf(FG_BRIGHT_YELLOW REVERSE "Done !\n" RESET);
	printf(FG_YELLOW REVERSE "hmap->top : %lu\n" RESET, hashmap->top);
	fgets(key, 255, stdin);
	//----------------------------------------------------------- input loop
	for (;;) {
		STOP_BENCH(repl);
		fputs("\x1b[102m > \x1b[0m", stdout);
		fgets(key, 255, stdin);
		size_t length = strlen(key);

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
			for (size_t k = 0; k < load_count; k++) {
				hmap_remove(hashmap, k);
			}
			printf(FG_BRIGHT_YELLOW REVERSE "hmap->top : %lu\n" RESET,
			       hashmap->top);
			continue;
		}

		//-------------------------------------------------- fill
		if ((strcmp(key, "fill")) == 0) {
			for (size_t k = 0; k < load_count; k++) {
				hmap_put(hashmap, k, k);
			}
			printf(FG_BRIGHT_YELLOW REVERSE "hmap->top : %lu\n" RESET,
			       hashmap->top);
			continue;
		}

		//-------------------------------------------------- dump
		if ((strcmp(key, "dump")) == 0) {
			dump_hashmap(hashmap);
			continue;
		}

		//-------------------------------------------------- sumb
		if ((strcmp(key, "sumb")) == 0) {
			sum_bucket(hashmap);
			continue;
		}

		//-------------------------------------------------- sum
		if ((strcmp(key, "sum")) == 0) {
			sum_store(hashmap);
			continue;
		}

		//-------------------------------------------------- find
		/**
		 * Test for values that do NOT exist with TEST_COUNT > load_count
		 */
		if ((strcmp(key, "find")) == 0) {
			sum_value = 0;
			for (size_t k = 0; k < TEST_COUNT; k++) {
				sum_value += hmap_get(hashmap, k);
			}
			printf("sum : %lu\n", sum_value);

			for (int k = TEST_COUNT - 1; k >= 0; k--) {
				sum_value -= hmap_get(hashmap, k);
			}
			printf("sum : %lu\n", sum_value);

			for (size_t k = TEST_COUNT; k > 0; k--) {
				size_t random_key = rand() % capacity;
				sum_value += hmap_get(hashmap, random_key);
			}
			printf("sum : %lu\nTEST_COUNT : %d\n", sum_value, TEST_COUNT);
			continue;
		}
		//-------------------------------------------------- findrand
		if ((strcmp(key, "findrand")) == 0) {
			sum_value = 0;

			for (size_t k = TEST_COUNT - 1; k > 0; k--) {
				size_t random_key = rand() % capacity;
				sum_value += hmap_get(hashmap, random_key);
			}
			printf("sum : %lu\nTEST_COUNT : %d\n", sum_value, TEST_COUNT);
			continue;
		}

		if ((strcmp(key, "add")) == 0) {
			printf("todo\n");
			continue;
		}

		//------------------------------------------------------- find / put
		if (key[0] != '\0') {
			char *key_end;
			size_t numeric_key;
			errno = 0;
			// Allow seed written in bases other than 10
			// Prefix with 0x for base 16
			// Prefix 0 for base 8
			numeric_key = strtoumax(key, &key_end, 0);

			// Abort on errors
			if ((errno != 0) && (numeric_key == 0)) {
				perror("Error : strtoumax () could NOT process given seed ");
				continue;
			}
			// Abort if seed is only junk or prefixed with junk
			// Ignore junk after valid seed though
			// Note : a leading 0 means base 8, thus 09 will yield 0 as 9
			//        is considered junk in base 8
			if (key_end == key) {
				printf("Error : could NOT process given seed : junk !\n");
				continue;
			}

			size_t result = hmap_find(hashmap, numeric_key);

			if (result == HMAP_NOT_FOUND) {
				puts("Key not found ! \n");
			}
			else {
				printf("Looking for %lu -> found @ %lu\n", numeric_key, result);
				printf("\t\t-> value : %lu\n", hmap_get(hashmap, numeric_key));
				printf("Removing entry !\n");
				hmap_remove(hashmap, numeric_key);
				printf(FG_YELLOW REVERSE "hmap->top : %lu\n" RESET,
				       hashmap->top);
			}
		}
	}

	//-------------------------------------------------------------- cleanup
	hmap_delete_hashmap(hashmap);
	return 0;
}