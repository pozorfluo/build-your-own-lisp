//------------------------------------------------------------------ SUMMARY ---
/**
 * Based on the work of the Abseil Team and publications of Daniel Lemire,
 * Peter Kankowski, Malte Skarupke, Maurice Herlihy, Nir Shavit, Moran Tzafrir,
 * Emmanuel Goossaert.
 *
 * Implement a hash map tuned for lispy / LispEnv
 *
 *
 * todo
 *   - [ ] Use fast range instead of modulo if tab_hash isn't fit, bitmask if
 *         desperate
 */

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

#ifdef DEBUG_HMAP
#include <math.h> /* pow() */
// #define DEBUG_MALLOC
#endif /* DEBUG_HMAP */

#include "debug_xmalloc.h"
//------------------------------------------------------------ MAGIC NUMBERS ---
#define PROBE_LENGTH 16

//------------------------------------------------------------------- MACROS ---
#define ARRAY_LENGTH(_array) (sizeof(_array) / sizeof((_array)[0]))

//------------------------------------------------------------- DECLARATIONS ---
typedef signed char meta_byte;
// typedef int16_t hop_map;

enum meta_ctrl {
	META_EMPTY    = -128, /* 0b10000000 */
	META_DELETED  = -2,   /* 0b11111110 */
	META_MARKED   = -1,   /* 0b11111111 */
	META_OCCUPIED = 0,    /* 0b0xxxxxxx */
	/* Think of META_OCCUPIED as anything like 0b0xxxxxxx */
	/* aka 0 <= META_OCCUPIED < 128 */
};

struct hmap_allocator {
	void *(*malloc)(size_t size);
	void (*free)(void *);
};

struct hmap_entry {
	char *key;
	void *value;
};

#ifdef DEBUG_HMAP
struct hmap_stats {
	size_t hashes_tally_or;
	size_t hashes_tally_and;
	size_t put_count;
	size_t collision_count;
	size_t hashes_ctrl_collision_count;
	size_t putfail_count;
	size_t swap_count;
	size_t del_count;
	size_t find_count;
	unsigned int n;
};
#endif /* DEBUG_HMAP */

struct hmap_buckets {
	meta_byte *metas;
	meta_byte *distances; /* this costs 1 byte per bucket, ~= 6 % increase */
	struct hmap_entry *entries;
};

/**
 * Check packing if necessary https://godbolt.org/z/e5Rryx
 */
struct hmap {
	/* todo
	 *   - [ ] assess how bad it is to haul 32 cachelines worth of xor_seed
	 *         with hmap
	 */
	struct hmap_buckets buckets;
	size_t actual_capacity; /* advertised capacity + PROBE_LENGTH */
	size_t capacity;        /* advertised capacity                */
	size_t count;           /* occupied entries count             */
	const struct hmap_allocator allocator;
	const size_t xor_seed[256];
#ifdef DEBUG_HMAP
	struct hmap_stats stats;
#endif /* DEBUG_HMAP */
};

struct hmap_init {
	struct hmap_buckets buckets;
	size_t actual_capacity;
	size_t capacity;
	size_t count;
	struct hmap_allocator allocator;
	size_t xor_seed[256];
#ifdef DEBUG_HMAP
	struct hmap_stats stats;
#endif /* DEBUG_HMAP */
};

//----------------------------------------------------- FORWARD DECLARATIONS ---
static inline size_t hash_tab(const unsigned char *key,
                              const size_t *const xor_seed)
    __attribute__((pure, always_inline));

static inline size_t hash_index(const size_t hash)
    __attribute__((const, always_inline));

static inline meta_byte hash_meta(const size_t hash)
    __attribute__((const, always_inline));

static inline int compare_keys(const char *const key_a, const char *const key_b)
    __attribute__((pure, always_inline));

static inline uint16_t probe_pattern(const meta_byte pattern,
                                     const meta_byte *const entry)
    __attribute__((pure, always_inline));

// static inline size_t hmap_find(const struct hmap *hashmap, const char *key)
//     __attribute__((const, always_inline));

static inline void xor_seed_fill(size_t *const xor_seed,
                                 const size_t hash_depth)
    __attribute__((always_inline));

static inline void destroy_entry(struct hmap *const hashmap, const size_t entry)
    __attribute__((always_inline));

//----------------------------------------------------------------- Function ---
/**
 * Compute a tabulation style hash for given key, xor_seed table
 *   -> hmap index
 *
 * todo
 *   - [ ] Assess hash function fitness
 *     + [x] Track stuck bits with hashes_tally_or, hashes_tally_and
       + [ ] Track control chunk collisions likelyhood with
             hashes_ctrl_collision_count
       + [ ] Look at SMHasher for more ways to assess fitness
         * [ ] See : https://github.com/aappleby/smhasher/wiki/SMHasher
 */
static inline size_t hash_tab(const unsigned char *key,
                              const size_t *const xor_seed)
{
	size_t hash = 0;

	while (*key) {
		hash ^= xor_seed[(*key + hash) & 0xFF] ^ *key;
		// #ifdef DEBUG_HMAP
		// 		printf(
		// 		    "partial hash : %lu\n"
		// 		    "char         : %c\n"
		// 		    "xor[char]    : %lu\n\n",
		// 		    hash,
		// 		    (char)*key,
		// 		    xor_seed[*key]);
		// #endif /* DEBUG_HMAP */
		key++;
	}

	return hash;
}

static inline size_t hash_index(const size_t hash) { return hash >> 7; }

static inline meta_byte hash_meta(const size_t hash) { return hash & 0x7F; }

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
 * Check if bucket at given index in given hashmap has had a deleted entry
 *   -> Truth value of predicate
 */
static inline int is_deleted(const meta_byte meta)
{
	return (meta == META_DELETED);
}

static inline int is_bucket_deleted(const struct hmap *const hashmap,
                                    const size_t index)
{
	return is_deleted(hashmap->buckets.metas[index]);
}

//----------------------------------------------------------------- Function ---
/**
 * Check if bucket at given index in given hashmap is marked as 'should not
 * stop an ongoing probe'
 *   -> Truth value of predicate
 */
static inline int is_marked(const meta_byte meta)
{
	return (meta == META_MARKED);
}

static inline int is_bucket_marked(const struct hmap *const hashmap,
                                   const size_t index)
{
	return is_marked(hashmap->buckets.metas[index]);
}

//----------------------------------------------------------------- Function ---
/**
 * Swap given entries
 *   -> nothing
 */
static inline void swap_entries(struct hmap *const hashmap,
                                const size_t a,
                                const size_t b)
{
	meta_byte tmp_meta;
	meta_byte tmp_distance;
	struct hmap_entry tmp_entry;

	tmp_meta     = hashmap->buckets.metas[a];
	tmp_distance = hashmap->buckets.distances[a];
	tmp_entry    = hashmap->buckets.entries[a];

	hashmap->buckets.metas[a]     = hashmap->buckets.metas[b];
	hashmap->buckets.distances[a] = hashmap->buckets.distances[b];
	hashmap->buckets.entries[a]   = hashmap->buckets.entries[b];

	hashmap->buckets.metas[b]     = tmp_meta;
	hashmap->buckets.distances[b] = tmp_distance;
	hashmap->buckets.entries[b]   = tmp_entry;

	return;
}
//----------------------------------------------------------------- Function ---
/**
 * Clobber given destination bucket with given source bucket
 *   -> nothing
 *
 * No questions asked !
 *
 * todo
 *   - [ ] Figure out how bad it is for clobber/slingshot operations that 3
 *         different arrays are touched
 *   - [ ] Decide if macros are worth the trouble
 *     + [ ] See : https://godbolt.org/z/ydwsK5
 */
static inline void clobber_bucket(struct hmap *const hashmap,
                                  const size_t dst,
                                  const size_t src)
{
	hashmap->buckets.metas[dst]     = hashmap->buckets.metas[src];
	hashmap->buckets.distances[dst] = hashmap->buckets.distances[src];
	hashmap->buckets.entries[dst]   = hashmap->buckets.entries[src];

	return;
}
static inline void slingshot(struct hmap *const hashmap,
                             const size_t dst,
                             const size_t src)
{
	size_t delta                    = dst - src;
	hashmap->buckets.metas[dst]     = hashmap->buckets.metas[src];
	hashmap->buckets.distances[dst] = hashmap->buckets.distances[src] + delta;
	hashmap->buckets.entries[dst]   = hashmap->buckets.entries[src];

	return;
}
/*
// #define CLOBBER_ENTRY(_hmap, _dst, _src) \
// 	do {                                                                       \
// 		_hmap->buckets.metas[_dst]     = _hmap->buckets.metas[_src];           \
// 		_hmap->buckets.distances[_dst] = _hmap->buckets.distances[_src];       \
// 		_hmap->buckets.keys[_dst]      = _hmap->buckets.keys[_src];            \
// 		_hmap->buckets.values[_dst]    = _hmap->buckets.values[_src];          \
// 	} while (0)
*/
//----------------------------------------------------------------- Function ---
/**
 * Clobber given destination bucket with given meta, distance, key, value
 *   -> nothing
 *
 * No questions asked !
 */
static inline void clobber_bucket_with(struct hmap *const hashmap,
                                       const size_t dst,
                                       const meta_byte meta,
                                       const meta_byte distance,
                                       char *const key,
                                       void *const value)
//    const struct hmap_entry entry)
{
	hashmap->buckets.metas[dst]         = meta;
	hashmap->buckets.distances[dst]     = distance;
	hashmap->buckets.entries[dst].key   = key;
	hashmap->buckets.entries[dst].value = value;

	return;
}
/*
// #define CLOBBER_ENTRY_WITH(_hmap, _dst, _meta, _distance, _key, _value) \
// 	do {                                                                       \
// 		_hmap->buckets.metas[_dst]     = _meta;                                \
// 		_hmap->buckets.distances[_dst] = _distance;                            \
// 		_hmap->buckets.keys[_dst]      = _key;                                 \
// 		_hmap->buckets.values[_dst]    = _value;                               \
// 	} while (0)
*/

//----------------------------------------------------------------- Function ---
/**
 * Compare given '\0' terminated string keys
 *   If length of given keys does NOT match
 *     -> -1
 *   Else
 *     If bytes comparison matches over length + 1
 *       -> 0
 *     Else
 *       -> difference
 */
static inline int compare_keys(const char *const key_a, const char *const key_b)
{
	size_t size_a  = strlen(key_a);
	size_t size_b  = strlen(key_b);
	int difference = -1;

	// do not bother with switch & jump, gcc outputs the same assembly
	// see : https://godbolt.org/z/suQQMk
	if (size_a == size_b) {
		difference = memcmp(key_a, key_b, size_a + 1);
	}
	return difference;
}

#ifdef DEBUG_HMAP
//----------------------------------------------------------------- Function ---
/**
 * Pretty print bits for given n bytes at given data pointer
 *   Handle little endian only
 *   -> nothing
 */
void print_bits(const size_t n, void const *const data)
{
	unsigned char *bit = (unsigned char *)data;
	unsigned char byte;

	for (int i = n - 1; i >= 0; i--) {
		for (int j = 7; j >= 0; j--) {
			byte = (bit[i] >> j) & 1;
			printf("%u", byte);
		}
	}

	putchar('\n');
}

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
	    "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x "
	    "%02x "
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
//----------------------------------------------------------------- Function ---
/**
 * Probe metadata chunk of 16 bytes for given meta_byte pattern starting at
 * given metadata entry
 *   -> Matches bitmask
 */
static inline uint16_t probe_pattern(const meta_byte pattern,
                                     const meta_byte *const entry_meta)
{
	/* setup filter */
	const __m128i filter = _mm_set1_epi8(pattern);

	/* filter chunks */
	const __m128i chunk = _mm_loadu_si128((__m128i *)(entry_meta));
	const __m128i match = _mm_cmpeq_epi8(filter, chunk);

#ifdef DEBUG_HMAP
	puts("probe_pattern filter :");
	print_m128i_hexu8(filter);
	print_m128i_hexu8(chunk);
#endif /* DEBUG_HMAP */

	return _mm_movemask_epi8(match);
}
//----------------------------------------------------------------- Function ---
/**
 * Probe metadata chunk of 16 bytes for META_EMPTY slots starting at given
 * metadata entry
 *   -> Matches bitmask
 */
// static inline uint16_t probe_empty(const meta_byte *const entry_meta)
// {
// 	/* setup filter */
// 	const __m128i filter = _mm_set1_epi8(META_EMPTY);

// 	/* filter chunks */
// 	const __m128i chunk = _mm_loadu_si128((__m128i *)(entry_meta));
// 	const __m128i match = _mm_cmpeq_epi8(filter, chunk);

// #ifdef DEBUG_HMAP
// 	puts("probe_empty filter :");
// 	print_m128i_hexu8(filter);
// 	print_m128i_hexu8(chunk);
// #endif /* DEBUG_HMAP */

// 	return _mm_movemask_epi8(match);
// }
//----------------------------------------------------------------- Function ---
/**
 * Probe distances chunk of 16 bytes for 'poor' entries relative to given
 * entry
 *   -> Matches bitmask
 *
 *   Build a filter of the projected distances from start of the probe
 *   Load a PROBE_LENGTH sized chunk of distance data from given entry
 *   Apply filter comparing if greater than
 *   Reduce to 16 bits bitmask
 *     ->  Matches bitmask
 */
static inline uint16_t probe_robin(const meta_byte *const entry_distance)
{
	/* setup filter */
	/* 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f */
	const __m128i filter = {0x0706050403020100, 0x0f0e0d0c0b0a0908};

	/* filter chunks */
	const __m128i chunk = _mm_loadu_si128((__m128i *)(entry_distance));
	const __m128i match = _mm_cmpgt_epi8(filter, chunk);

#ifdef DEBUG_HMAP
	puts("probe_robin filter :");
	print_m128i_hexu8(filter);
	print_m128i_hexu8(chunk);
#endif /* DEBUG_HMAP */
	return _mm_movemask_epi8(match);
}
//----------------------------------------------------------------- Function ---
/**
 * Probe distances chunk of 16 bytes for entries related to given
 * entry, ie., entries with distances that exactly points back to given entry
 *   -> Matches bitmask
 *
 *   Build a filter of the projected distances from start of the probe
 *   Load a PROBE_LENGTH sized chunk of distance data from given entry
 *   Apply filter comparing if equal
 *   Reduce to 16 bits bitmask
 *     ->  Matches bitmask
 */
static inline uint16_t probe_related(const meta_byte *const entry_distance)
{
	/* setup filter */
	/* 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f */
	const __m128i filter = {0x0706050403020100, 0x0f0e0d0c0b0a0908};

	/* filter chunks */
	const __m128i chunk = _mm_loadu_si128((__m128i *)(entry_distance));
	const __m128i match = _mm_cmpeq_epi8(filter, chunk);

#ifdef DEBUG_HMAP
	puts("probe_related filter :");
	print_m128i_hexu8(filter);
	print_m128i_hexu8(chunk);
#endif /* DEBUG_HMAP */
	return _mm_movemask_epi8(match);
}

//----------------------------------------------------------------- Function ---
/**
 * Locate given key in given hmap
 * If given key exists
 *   -> entry index
 * Else
 *   -> out of bound value ( > actual_capacity)
 *
 * todo
 *   - [x] Probe for empty slots
 *   - [x] Consider next chunk only if previous probe was completely occupied
 *   - [x] Wrap to the beginning of the table
 *   - [x] Give up once all chunks are scanned
 *   - [x] Enforce lazy branching and computations to keep ideal path pristine
 *   - [ ] Investigate ways to probe for Match or META_EMPTY at once
 */
size_t hmap_find(const struct hmap *const hashmap, const char *const key)
{
	size_t hash    = hash_tab((unsigned char *)key, hashmap->xor_seed);
	size_t index   = hash_index(hash);
	meta_byte meta = hash_meta(hash);

	// #ifdef DEBUG_HMAP
	// 	hashmap->stats.hashes_tally_or |= hash;
	// 	hashmap->stats.hashes_tally_and &= hash;
	// #endif /* DEBUG_HMAP */

	meta_byte *chunk;
	uint16_t match_mask;
	size_t chunk_count = 0;

	do {
		chunk      = hashmap->buckets.metas + index;
		match_mask = probe_pattern(meta, chunk);

#ifdef DEBUG_HMAP
		print_bits(2, &match_mask);
#endif /* DEBUG_HMAP */

		/* loop through set bit in bitmask to access matches */
		/**
		 * todo
		 *   - [ ] Check how bad it is that it may run once with
		 *         match_mask == 0 on first go around
		 */
		do {
			// while (match_mask != 0) {
			// const size_t offset    = __builtin_ctz(match_mask);
			const size_t offset = _bit_scan_forward(match_mask);
			size_t match           = index + offset;
			const char *probed_key = hashmap->buckets.entries[match].key;

			/* Can an entry that yields a match ever have a NULL key ? */
			if ((probed_key != NULL) && (compare_keys(probed_key, key)) == 0) {
				// hashmap->stats.find_count++;
				/* Found key ! */
				return match;
			}
			/* remove least significant set bit */
			match_mask ^= match_mask & (-match_mask);
		} while (match_mask != 0);
		/* no match in current chunk */
		/* chunk done */
		chunk_count++;

		/* if there is any empty slot in the chunk that was probed */
		/* no need to check the next */
		/* -> key does NOT exist */
		if (probe_pattern(META_EMPTY, chunk)) {
			break;
		};

		/* THIS SHOULD HARDLY EVER BE REACHED */
		if ((index += PROBE_LENGTH) > hashmap->capacity) {
			/* wrap around the table */
			index = 0;
		};
	} while ((chunk_count * PROBE_LENGTH) < hashmap->actual_capacity);

#ifdef DEBUG_HMAP
	printf("Key not found after probing %lu chunks out of %lu\n",
	       chunk_count,
	       hashmap->actual_capacity / 16);
#endif /* DEBUG_HMAP */

	/* key not found */
	return hashmap->actual_capacity + 1;
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
                                        const char *const key,
                                        size_t index,
                                        const meta_byte meta)
{
	meta_byte *chunk;
	uint16_t match_mask;
	size_t chunk_count = 0;

	do {
		chunk      = hashmap->buckets.metas + index;
		match_mask = probe_pattern(meta, chunk);

#ifdef DEBUG_HMAP
		print_bits(2, &match_mask);
#endif /* DEBUG_HMAP */

		/* loop through set bit in bitmask to access matches */
		while (match_mask != 0) {
			// const size_t offset    = __builtin_ctz(match_mask);
			const size_t offset = _bit_scan_forward(match_mask);

			/* Can an entry that yields a match ever have a NULL key ? */
			size_t match = index + offset;
			if ((compare_keys(hashmap->buckets.entries[match].key, key)) == 0) {
				// hashmap->stats.find_count++;
				/* Found key ! */
				return match;
			}
			/* clear least significant set bit */
			match_mask ^= match_mask & (-match_mask);
		}
		/* no match in current chunk */
		/* chunk done */
		chunk_count++;

		/* if there is any empty slot in the chunk that was probed */
		/* no need to check the next */
		if ((match_mask = probe_pattern(META_EMPTY, chunk))) {
			/* -> key does NOT exist, insert in first empty slot */
			return index + _bit_scan_forward(match_mask);
		};

		/* THIS SHOULD HARDLY EVER BE REACHED */
		if ((index += PROBE_LENGTH) > hashmap->capacity) {
			/* wrap around the table */
			index = 0;
		};
	} while ((chunk_count * PROBE_LENGTH) < hashmap->actual_capacity);

#ifdef DEBUG_HMAP
	printf("Key not found after probing %lu chunks out of %lu\n",
	       chunk_count,
	       hashmap->actual_capacity / 16);
#endif /* DEBUG_HMAP */

	/* key not found, no empty slots left */
	return hashmap->actual_capacity + 1;
}

//----------------------------------------------------------------- Function ---
/**
 * Locate given value for given key in given hmap
 * If given key exists
 *   -> pointer to value in hmap
 * Else
 *   -> NULL
 */
void *hmap_get(const struct hmap *const hashmap, const char *const key)
{
	const size_t entry = hmap_find(hashmap, key);
	void *value        = NULL;

	if (entry <= hashmap->actual_capacity) {
		value = hashmap->buckets.entries[entry].value;
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
 *   Probe for robin hood 'swaps' till empty slot
 *   Hold the new entry, DO NOT SWAP
 *   Clobber empty slot with last swap candidate entry
 *   Go upstream in the swap candidates list and push it to clobber next
 *   entry
 *   Clobber most upstream swap candidate with new entry
 *     -> new entry index
 *
 * note
 *   Do not allow a full table, resize before
 *     -> hmap_put does not handle inserting on a full table
 *
 * todo
 *   - [ ] Keep track of entries distance from @home, min, max, average
 *   - [ ] Keep track of theoritical robin hood "swaps"
 *   - [ ] Keep track of actual clobber operation executed instead of swaps
 *   - [ ] Add hmap fitness / resize logic
 *   - [ ] Consider _mm_shuffle_epi8 to execute the swaps for metas and distance
 *   - [ ] Decide what happens to the existing key when updating
 *     + [x] Free either new or existing if they do NOT alias ?
 *     + [ ] Switch to allocing on put ?
 *     + [ ] Leave the responsibility to the user to handle its frees
 *     + [ ] Consider similar problem with values, especially since updating
 *           is sort of 'silent' and updating means clobbering a pointer to
 *           alloced memory
 *     + [ ] Look at how libcfu, build your own lisp, deal with this type of
 *           scenario
 *     + [ ] Look at String Interning techniques
 *   - [ ] Reconsider this shelved version :
 *         Probe for robin hood 'swaps' till empty slot
 *
 *          Look at hits from original probe_robin for next swaps
 *          No other have to be considered, correct ?
 *
 *         If you hit empty slot in this probe
 *            start furthest downstream
 *            push each hit in mask_match to next hit toward empty
 *         slot
 *         else
 *            same thing but furthest downstream replace temporarily
 *            held new entry
 *
 *         Hold the new entry, DO NOT SWAP
 *         Clobber empty slot with last swap candidate entry
 *         Go upstream in the swap candidates list and push it to clober
 *         next entry
 *         Clobber most upstream swap candidate with new entry
 *         -> new entry index
 */
size_t hmap_put(struct hmap *const hashmap, char *const key, void *value)
{
	assert(key != NULL);
	assert(value != NULL);

	/* Prepare temp entry */
	const size_t hash    = hash_tab((unsigned char *)key, hashmap->xor_seed);
	const size_t home    = hash_index(hash);
	const meta_byte meta = hash_meta(hash);

	/* Find given key or first empty slot */
	size_t candidate = hmap_find_or_empty(hashmap, key, home, meta);

	//----------------------------------------------------- empty slot found
	if (is_empty(hashmap->buckets.metas[candidate])) {
		/**
		 * Thierry La Fronde method : Slingshot the rich !
		 *
		 * Go backwards from candidate empty bucket to home
		 *   If current entry distance is lower or equal than the entry upstream
		 *     Slingshot it to candidate empty bucket !
		 *     Make current entry the new candidate empty bucket
		 * Slingshot given entry to wherever candidate empty bucket is
		 * Increment hmap entry count
		 *
		 * note
		 * 	 Find returns existing key or first empty bucket
		 *   By definition there should be no other empty bucket between home
		 *   and candidate bucket
		 *   Distance to home is stored on a single byte which means there
		 *   can NOT be more than 256 slingshot jobs to resolve. This is
		 *   probably overly generous already.
		 * todo
		 *   - [ ] Handle cases where find wrapped around the table
		 *   - [ ] Hold a rich target list as you update distance in the TLF
		 *         loop
		 *     + [ ] Unwind and carry out the slingshot when done
		 */
		// int rich_targets[256];

		for (size_t bucket = candidate; bucket != home; bucket--) {
			if (hashmap->buckets.distances[bucket] <=
			    hashmap->buckets.distances[bucket - 1]) {
				slingshot(hashmap, candidate, bucket);
				candidate = bucket;
#ifdef DEBUG_HMAP
				printf("Slingshot %lu to %lu\n", bucket, candidate);
				hashmap->stats.swap_count++;
#endif /* DEBUG_HMAP */
			}
		}
		/**
		 * note
		 *   this says implicitely that current implementation cannot deal with
		 *   chains over 256
		 */
		// meta_byte distance   = candidate - home;
		clobber_bucket_with(
		    hashmap, candidate, meta, candidate - home, key, value);
		hashmap->count++;
#ifdef DEBUG_HMAP
		/**
		 * this is NOT the collision metric to assess hash function
		 * It counts the numbers of times a new key did NOT end up in its home
		 * bucket wether it is due to a direct hash collision or because of
		 * an unrelated key ended up in its home bucket
		 */
		if ((candidate - home) != 0) {
			hashmap->stats.collision_count++;
		}
#endif /* DEBUG_HMAP */
	}
	//------------------------------------------------------ given key found
	else {
		/**
		 * If given key and existing key pointer do NOT alias
		 *    Free existing, use new one
		 */
		if (key != hashmap->buckets.entries[candidate].key) {
			// Use XFREE macro instead of destructor while hmap is a WIP
			XFREE(hashmap->buckets.entries[candidate].key,
			      "hmap_put : dupe of existing key");
			hashmap->buckets.entries[candidate].key = key;
		}
		/**
		 * If given value and existing value pointer do NOT alias
		 *    Free existing, use new one
		 */
		if (value != hashmap->buckets.entries[candidate].value) {
			// Use XFREE macro instead of destructor while hmap is a WIP
			XFREE(hashmap->buckets.entries[candidate].value,
			      "hmap_put : dupe of existing value");
			hashmap->buckets.entries[candidate].value = value;
		}
	}

#ifdef DEBUG_HMAP
	hashmap->stats.hashes_tally_or |= hash;
	hashmap->stats.hashes_tally_and &= hash;
#endif /* DEBUG_HMAP */

	/* -> new or updated entry index */
	return candidate;
}

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
static inline void destroy_entry(struct hmap *const hashmap, const size_t entry)
{
	// Use XFREE macro instead of destructor while hmap is a WIP
	XFREE(hashmap->buckets.entries[entry].key,
	      hashmap->buckets.entries[entry].key);
	// destructor(hashmap->buckets.keys[entry]);
	XFREE(hashmap->buckets.entries[entry].value, "destroy_entry");
	return;
}
//----------------------------------------------------------------- Function ---
/**
 * Look for given key in given hmap
 * If given key exists
 *   Destroy entry associated to given key
 *   Mark metadata associated to given key as CTRL_DELETED
 *   Update given hmap stats
 *   -> destroyed entry index
 * Else
 *   -> out of bound value ( > actual_capacity )
 */
size_t hmap_remove(struct hmap *const hashmap, const char *const key)
{
	size_t entry = hmap_find(hashmap, key);

	if (entry < hashmap->actual_capacity) {
		destroy_entry(hashmap, entry);
		// hashmap->buckets.metas[entry] = META_DELETED; //|=META_DELETED;
		// useful ?
		hashmap->buckets.metas[entry] = META_EMPTY; //|=META_DELETED; useful ?
		hashmap->count--;
	}

#ifdef DEBUG_HMAP
	hashmap->stats.del_count++;
#endif /* DEBUG_HMAP */
	return entry;
}

//----------------------------------------------------------------- Function ---
/**
 * Free given Hashmap
 *   -> nothing
 *
 * todo
 *   - [ ] Consider a clear_hashmap func : destroy entries only
 *   - [ ] Consider a reset_hashmap func : destroy entries, reset hashmap to
 *         given new size
 *   - [ ] Use the probe
 */
void hmap_delete_hashmap(struct hmap *const hashmap)
{
	for (size_t bucket = 0; bucket < hashmap->actual_capacity; bucket++) {
		if (is_occupied(hashmap->buckets.metas[bucket])) {
			destroy_entry(hashmap, bucket);
#ifdef DEBUG_HMAP
			hashmap->stats.del_count++;
#endif /* DEBUG_HMAP */
		}
	}
	// stats_hashmap(hashmap);
	XFREE(hashmap->buckets.metas, "delete_hashmap hashmap->buckets.metas");
	XFREE(hashmap->buckets.distances,
	      "delete_hashmap hashmap->buckets.distances");
	XFREE(hashmap->buckets.entries, "delete_hashmap hashmap->buckets.entries");
	XFREE(hashmap, "delete_hashmap");
}

//----------------------------------------------------------------- Function ---
/**
 * Fill given xor_seed for given hash depth
 *   -> pointer to new xor_seed
 */
static inline void xor_seed_fill(size_t *const xor_seed,
                                 const size_t hash_depth)
{
	srand(__rdtsc());
	for (size_t i = 0; i < 256; i++) {
		// % capacity is okay-ish as long as its a power of 2
		// Expect skewed distribution otherwise
		xor_seed[i] = rand() % (hash_depth);
	}
}

//----------------------------------------------------------------- Function ---
/**
 * Create a new hmap of capacity equal to 2^(given n),  with given seed
 *
 * 	 Init allocator to custom given allocator or default if none given
 *   Enforce n >= 1
 *   Init a dummy xor_seed with hash_depth set to yield n + 7 ctrl bits
 *   Init a dummy hmap_init
 * 	 Copy dummy xor_seed in dummy HashmapInit
 *   Allocate a Hashmap
 *     If allocation fails,  clean up and abort
 *     -> NULL
 *   Copy dummy HashmapInit to Hashmap
 *   Allocate Hashmap buckets
 *     If any allocation fails, clean up and abort
 *     -> NULL
 *   Init Hashmap buckets
 *     -> pointer to new hmap
 *
 * todo
 *   - [ ] Replace parameter n with requested_capacity
 *     + [ ] Compute next 2^(n) - 1 from requested_capacity
 *     + [x] Store n in debug stats
 *   - [ ] Make sure hashmap_init cannot blow up the stack on HUGE sizes
 *     + [ ] Handle such cases if any with branching to an alloc
 *     + [ ] Consider dropping the whole hashmap_init step and the const
 *           qualifier on xor_seed_init as workaround
 *   - [ ] Consider one big alloc of
 *         sizeof(metas +  keys + values ) * actual_capacity
 *     + [ ] Consider setting pointers inside the struct accordingly
 *           after the one big alloc
 */

#define MALLOC_BUCKETS_ARRAY(_array, _capacity)                                \
	do {                                                                       \
		_array =                                                               \
		    XMALLOC(sizeof(*(_array)) * _capacity, "new_hashmap", #_array);    \
		if (_array == NULL) {                                                  \
			goto err_free_buckets;                                             \
		}                                                                      \
	} while (0)

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
	/* tab_hash requires a mininum of 8 bits of hash space */
	/* including the 7 bits required for ctrl_byte */
	const size_t capacity   = (n < 1) ? (1u << 1) : (1u << n);
	const size_t hash_depth = capacity << 7;

	//---------------------------------------------------- xor_seed init
	size_t xor_seed_init[256];
	xor_seed_fill(xor_seed_init, hash_depth);

	//-------------------------------------------------------- hmap init
	const size_t actual_capacity = capacity + PROBE_LENGTH;

#ifdef DEBUG_HMAP
	struct hmap_stats stats_init  = {0, SIZE_MAX, 0, 0, 0, 0, 0, 0, 0, n};
	struct hmap_init hashmap_init = {
	    {NULL}, actual_capacity, capacity, 0, allocator_init, {0}, stats_init};
#else
	struct hmap_init hashmap_init = {
	    {NULL}, actual_capacity, capacity, 0, allocator_init, {0}};
#endif /* DEBUG_HMAP */

	memcpy(hashmap_init.xor_seed, xor_seed_init, sizeof(xor_seed_init));

	struct hmap *new_hashmap =
	    XMALLOC(sizeof(struct hmap), "new_hashmap", "new_hashmap");
	memcpy(new_hashmap, &hashmap_init, sizeof(struct hmap));
	if (new_hashmap == NULL) {
		goto err_free_hashmap;
	}

	MALLOC_BUCKETS_ARRAY(new_hashmap->buckets.metas, actual_capacity);
	MALLOC_BUCKETS_ARRAY(new_hashmap->buckets.distances, actual_capacity);
	MALLOC_BUCKETS_ARRAY(new_hashmap->buckets.entries, actual_capacity);

	/* The value is passed as an int, but the function fills the block of
	 * memory using the unsigned char conversion of this value */
	memset(new_hashmap->buckets.metas,
	       META_EMPTY,
	       sizeof(*(new_hashmap->buckets.metas)) * actual_capacity);
	memset(new_hashmap->buckets.distances,
	       0,
	       sizeof(*(new_hashmap->buckets.metas)) * actual_capacity);
	/* Is this enough to be able to check if ptr == NULL ? */
	memset(new_hashmap->buckets.entries,
	       0,
	       sizeof(*(new_hashmap->buckets.entries)) * actual_capacity);

	return new_hashmap;

/* free(NULL) is ok, correct ? */
err_free_buckets:
	XFREE(new_hashmap->buckets.metas, "new_hashmap->buckets.metas");
	XFREE(new_hashmap->buckets.distances, "new_hashmap->buckets.distances");
	XFREE(new_hashmap->buckets.entries, "new_hashmap->buckets.entries");
err_free_hashmap:
	XFREE(new_hashmap, "new_hashmap");
	return NULL;
}

#ifdef DEBUG_HMAP
//----------------------------------------------------------------- Function ---
/**
 * Compute the expected number of entries per bucket for given number of
 * keys,
 * buckets
 *   -> expected number of entries per bucket
 */
double expected_entries_per_bucket(const size_t keys, const size_t buckets)
{
	return (double)keys / (double)buckets;
}
//----------------------------------------------------------------- Function ---
/**
 * Compute the expected number of empty buckets for given number of keys,
 * buckets
 *   -> expected number of empty buckets
 */
double expected_empty_buckets(const size_t keys, const size_t buckets)
{
	return (double)buckets * pow(1 - (1 / (double)buckets), (double)keys);
}
//----------------------------------------------------------------- Function ---
/**
 * Compute the expected number of filled buckets for given number of keys,
 * buckets
 *   -> expected number of filled buckets
 */
double expected_filled_buckets(const size_t keys, const size_t buckets)
{
	return (double)buckets - expected_empty_buckets(keys, buckets);
}
//----------------------------------------------------------------- Function ---
/**
 * Compute the expected number of collisions for given number of keys,
 * buckets
 *   -> expected number of collisions
 */
double expected_collisions(const size_t keys, const size_t buckets)
{
	return (double)keys - expected_filled_buckets(keys, buckets);
}
//----------------------------------------------------------------- Function ---
/**
 * Pretty print xor_seed for given Hashmap
 *   -> nothing
 */
void print_xor_seed(const struct hmap *const hashmap)
{
	puts("xor_seed            :");
	for (size_t i = 0; i < 256; i++) {
		printf("%0*lx ",
		       (int)ceil(((hashmap->stats.n + 7) / 4.0)),
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
void stats_hashmap(const struct hmap *const hashmap)
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

	printf("hashmap->stats.n          : %u\n", hashmap->stats.n);
	printf("hashmap->capacity   : %lu / %lu\n",
	       hashmap->capacity,
	       hashmap->actual_capacity);
	printf("hashmap->count      : %lu \t-> %f%%\n",
	       hashmap->count,
	       (double)hashmap->count / (double)hashmap->capacity * 100);
	printf("hashmap->stats.collision_count  : %lu \t-> %lf%%\n",
	       hashmap->stats.collision_count,
	       (double)hashmap->stats.collision_count / (double)hashmap->count *
	           100);
	// printf("hashmap->stats.hashes_ctrl_collision_count  : %lu\n",
	//        hashmap->stats.hashes_ctrl_collision_count);

	printf("hashmap->stats.hashes_tally_or  : %lu\n",
	       hashmap->stats.hashes_tally_or);
	printf("hashmap->stats.hashes_tally_and : %lu\n",
	       hashmap->stats.hashes_tally_and);
	// printf("hashmap->stats.put_count        : %lu\n",
	// hashmap->stats.put_count);
	// printf("hashmap->stats.putfail_count    : %lu\n",
	//        hashmap->stats.putfail_count);
	printf("hashmap->stats.swap_count       : %lu\n",
	       hashmap->stats.swap_count);
	// printf("hashmap->stats.del_count        : %lu\n",
	// hashmap->stats.del_count);
	// printf("hashmap->stats.find_count       : %lu\n",
	//        hashmap->stats.find_count);

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
void dump_hashmap(const struct hmap *const hashmap)
{
	size_t empty_bucket = 0;
	int max_distance    = 0;

	for (size_t i = 0; i < hashmap->actual_capacity; i++) {
		if (hashmap->buckets.entries[i].key == NULL) {
			empty_bucket++;
			printf("\x1b[100mhashmap->\x1b[30mbucket[%lu]>> EMPTY <<\x1b[0m\n",
			       i);
			continue;
		}

		/* Color code distance from home using ANSI esc code values */
		int colour;

		switch (hashmap->buckets.distances[i]) {
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
		max_distance = (hashmap->buckets.distances[i] > max_distance)
		                   ? hashmap->buckets.distances[i]
		                   : max_distance;
		printf("\x1b[10%dmhashmap->\x1b[30mbucket[%lu]\x1b[0m>>", colour, i);

		// printf("\t %016lx : %s\n",
		printf(" %lu[%d] : %s\n",
		       hash_index(
		           hash_tab((unsigned char *)hashmap->buckets.entries[i].key,
		                    hashmap->xor_seed)),
		       hashmap->buckets.distances[i],
		       hashmap->buckets.entries[i].key);
	}

	printf("empty_buckets       : %lu \t-> %f%%\n",
	       empty_bucket,
	       (double)empty_bucket / (double)hashmap->capacity * 100);
	printf("max_distance        : %d\n", max_distance);

#ifdef DEBUG_HMAP
	stats_hashmap(hashmap);
	print_xor_seed(hashmap);
#endif /* DEBUG_HMAP */
}

//--------------------------------------------------------------------- MAIN
//---
int main(void)
{
	puts(
	    "todo\n"
	    "\t- [ ] Implement baseline non-SIMD linear probing\n"
	    "\t\t+ [ ] Benchmark against SIMD wip versions\n"
	    "\t- [ ] Implement the most basic put operation to mock tables\n");
	// uint32_t seed = 31;
	size_t n = 8;
	// Hashmap *hashmap = new_hashmap(n, seed, hash_fimur_reduce);

	fputs(FG_BRIGHT_BLUE REVERSE
	      "Table size is 2^n. Enter n ( default n=8 ) " FG_BRIGHT_BLUE
	      " ? " RESET,
	      stdout);
	scanf("%lu", &n);

	struct hmap *const hashmap = hmap_new(n, NULL);
//----------------------------------------------------------- mock entry

//-------------------------------------------------------------------- setup
#define KEYPOOL_SIZE 32
	char random_keys[KEYPOOL_SIZE] = {'\0'};
	size_t test_count              = (1 << n) * 0.75; // 1 << (n - 1);
	char key[256];
	char *dummy_key   = NULL;
	char *dummy_value = NULL;

	srand(__rdtsc());

	for (size_t k = 0; k < test_count; k++) {
		for (size_t i = 0; i < KEYPOOL_SIZE - 1; i++) {
			random_keys[i] = (char)(rand() % 26 + 0x61); //% 95 + 0x20);
			// putchar(random_keys[i]);
		}
		// putchar('\n');

		dummy_key   = strdup(&random_keys[rand() % (KEYPOOL_SIZE - 1)]);
		dummy_value = strdup(&random_keys[rand() % (KEYPOOL_SIZE - 1)]);

#ifdef DEBUG_HMAP
		// hmap_put(hashmap, dummy_key, dummy_value);
		const size_t hash =
		    hash_tab((unsigned char *)dummy_key, hashmap->xor_seed);
		const size_t home    = hash_index(hash);
		const meta_byte meta = hash_meta(hash);
		printf(
		    "%s\n"
		    "\tdummy_value : %s\n"
		    "\thash        : %lu\n"
		    "\thome        : %lu\n"
		    "\tmeta        : %d\n",
		    dummy_key,
		    dummy_value,
		    hash,
		    home,
		    meta);
#endif /* DEBUG_HMAP */

		hmap_put(hashmap, dummy_key, dummy_value);
	}

	//----------------------------------------------------------- input loop
	for (;;) {
		fputs("\x1b[102m > \x1b[0m", stdout);
		// scanf("%s", key);
		fgets(key, 255, stdin);
		size_t length = strlen(key);

		/* trim newline */
		if ((length > 0) && (key[--length] == '\n')) {
			key[length] = '\0';
		}

		if ((strcmp(key, "exit")) == 0) {
			break;
		}

		if ((strcmp(key, "dump")) == 0) {
			dump_hashmap(hashmap);
			continue;
		}

#ifdef DEBUG_HMAP
		if ((strcmp(key, "seed")) == 0) {
			print_xor_seed(hashmap);
			continue;
		}
		if ((strcmp(key, "stats")) == 0) {
			stats_hashmap(hashmap);
			continue;
		}
#endif /* DEBUG_HMAP */

		if ((strcmp(key, "add")) == 0) {
			printf("todo\n");
			continue;
		}

		//------------------------------------------------------- find / put
		if (key[0] != '\0') {
			printf("Looking for %s -> found @ %lu\n",
			       key,
			       hmap_find(hashmap, key));
		}
	}

	//-------------------------------------------------------------- cleanup
	hmap_delete_hashmap(hashmap);
	return 0;
}