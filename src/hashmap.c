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
#endif

#include "ansi_esc.h"

#include <assert.h>
// #define DEBUG_HMAP

#ifdef DEBUG_HMAP
#include <math.h> /* pow() */
#define DEBUG_MALLOC
#endif /* DEBUG_HMAP */

#include "debug_xmalloc.h"

//---------------------------------------------------------------- BENCHMARK ---
#define BENCHMARK

#ifdef BENCHMARK
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

// #include <x86intrin.h>

// #define LOOP_SIZE 10000000

#define START_BENCH(_start)                                                    \
	do {                                                                       \
		_start = (float)clock() / CLOCKS_PER_SEC;                              \
	} while (0)

#define STOP_BENCH(_start, _stop, _diff, _result)                              \
	do {                                                                       \
		_stop   = (float)clock() / CLOCKS_PER_SEC;                             \
		_diff   = _stop - _start;                                              \
		_result = _diff;                                                       \
		printf("|><| %f s\n", _result);                                        \
	} while (0)

// printf(FG_YELLOW REVERSE "|><|" RESET FG_YELLOW " %f s\n" RESET,
//        result);

#else
#define LOOP_SIZE 0
#define START_BENCH(_start)
#define STOP_BENCH(_start, _stop, _diff, _result)
#define BENCH(_expression, _loop, _result)
#endif /* BENCHMARK */
//------------------------------------------------------------ MAGIC NUMBERS ---
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
#include <immintrin.h>
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
#define PROBE_LENGTH 16
#endif /* __AVX__ */

//------------------------------------------------------------------- MACROS ---
#define ARRAY_LENGTH(_array) (sizeof(_array) / sizeof((_array)[0]))

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
	size_t n;
};
#endif /* DEBUG_HMAP */

/**
 * Check packing https://godbolt.org/z/dvdKqM
 */
struct hmap {
	struct hmap_buckets buckets;
	struct hmap_entry *store;
	size_t top;
	size_t hash_shift; /* shift amount necessary for desired hash depth */
	size_t capacity;   /* now is actual capacity */
	size_t count;      /* occupied entries count */
#ifdef DEBUG_HMAP
	struct hmap_stats stats;
#endif /* DEBUG_HMAP */
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
 * Clobber given destination bucket with given source bucket
 *   -> nothing
 *
 * No questions asked !
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
                                       const size_t entry)
{
	hashmap->buckets.metas[dst]     = meta;
	hashmap->buckets.distances[dst] = distance;
	hashmap->buckets.entries[dst]   = entry;

	return;
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
#ifdef __AVX__
static inline uint32_t probe_pattern(const meta_byte pattern,
                                     const meta_byte *const entry_meta)
{
	/* setup filter */
	const __m256i filter = _mm256_set1_epi8(pattern);

	/* filter chunks */
	const __m256i chunk = _mm256_loadu_si256((__m256i *)(entry_meta));
	const __m256i match = _mm256_cmpeq_epi8(filter, chunk);

#ifdef DEBUG_HMAP
	puts("probe_pattern filter :");
	print_m128i_hexu8(filter);
	print_m128i_hexu8(chunk);
#endif /* DEBUG_HMAP */

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

#ifdef DEBUG_HMAP
	puts("probe_pattern filter :");
	print_m128i_hexu8(filter);
	print_m128i_hexu8(chunk);
#endif /* DEBUG_HMAP */

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
 *   - [ ] Consider using SIZE_MAX directly as an error
 *     + [ ] Add exists() inlinable function
 */
size_t hmap_find(const struct hmap *const hashmap, const size_t key)
{
	size_t hash    = reduce_fibo(key, hashmap->hash_shift);
	size_t index   = hash_index(hash);
	meta_byte meta = hash_meta(hash);

// #ifdef DEBUG_HMAP
// 	hashmap->stats.hashes_tally_or |= hash;
// 	hashmap->stats.hashes_tally_and &= hash;
// #endif /* DEBUG_HMAP */

#ifdef __AVX__
	uint32_t match_mask;
#else
	uint16_t match_mask;
#endif /* __AVX__ */

	meta_byte *chunk;
	size_t chunk_count = 0;

	do {
		chunk      = hashmap->buckets.metas + index;
		match_mask = probe_pattern(meta, chunk);

#ifdef DEBUG_HMAP
		print_bits(2, &match_mask);
#endif /* DEBUG_HMAP */

		/* loop through set bit in bitmask to access matches */
		while (match_mask != 0) {
			const size_t offset = __builtin_ctzl(match_mask); // uint32_t
			size_t match        = index + offset;
			const size_t probed_key =
			    hashmap->store[(hashmap->buckets.entries[match])].key;

			if (probed_key == key) {
				// hashmap->stats.find_count++;
				/* Found key ! */
				return match;
			}
			/* remove least significant set bit */
			match_mask ^= match_mask & (-match_mask);
		}
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
		if ((index += PROBE_LENGTH) > (hashmap->capacity - PROBE_LENGTH)) {
			/* wrap around the table */
			index = 0;
		};
	} while ((chunk_count * PROBE_LENGTH) < hashmap->capacity);

#ifdef DEBUG_HMAP
	printf("Key not found after probing %lu chunks out of %lu\n",
	       chunk_count,
	       hashmap->capacity / 16);
#endif /* DEBUG_HMAP */

	/* key not found */
	return hashmap->capacity + 1;
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
	size_t chunk_count = 0;

	do {
		chunk      = hashmap->buckets.metas + index;
		match_mask = probe_pattern(meta, chunk);

#ifdef DEBUG_HMAP
		print_bits(2, &match_mask);
#endif /* DEBUG_HMAP */

		/* loop through set bit in bitmask to access matches */
		while (match_mask != 0) {
			const size_t offset = __builtin_ctzl(match_mask); // uint32_t
			size_t match        = index + offset;
			const size_t probed_key =
			    hashmap->store[(hashmap->buckets.entries[match])].key;

#ifdef DEBUG_HMAP
			printf(
			    "comparing store key : %s\n"
			    "      and given key : %s\n",
			    probed_key,
			    key);
#endif /* DEBUG_HMAP */

			if (probed_key == key) {
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
			return index + __builtin_ctzl(match_mask);
		};

		/* THIS SHOULD HARDLY EVER BE REACHED */
		if ((index += PROBE_LENGTH) > (hashmap->capacity - PROBE_LENGTH)) {
			/* wrap around the table */
			index = 0;
		};
	} while ((chunk_count * PROBE_LENGTH) < hashmap->capacity);

#ifdef DEBUG_HMAP
	printf("Key not found after probing %lu chunks out of %lu\n",
	       chunk_count,
	       hashmap->capacity / 16);
#endif /* DEBUG_HMAP */

	/* key not found, no empty slots left */
	return hashmap->capacity + 1;
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
size_t hmap_get(const struct hmap *const hashmap, const size_t key)
{
	const size_t entry = hmap_find(hashmap, key);
	// void *value        = NULL;
	size_t value = 0;

	if (entry <= hashmap->capacity) {
		// value = hashmap->store[entry].value;
		value = hashmap->store[(hashmap->buckets.entries[entry])].value;
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
 *   - [ ] Handle cases where find wrapped around the table, either :
 *     + [ ] Use PROBE_LENGTH as max distance and forbid wrapping around
 *       * [ ] Trigger resize when distance above PROBE_LENGTH is
 *             requested
 *     + [ ] Add necessary tests and jumps to wrap around in put_hashmap
 *           when find return a position that wrapped around the map
 *     + [ ] Evaluate usefulness of wrapping around
 *     + [ ] Benchmark
 *   - [ ] Slingshot as you move backward then
 *     + [ ] Benchmark against :
 *       *  [ ] Hold a rich target list as you update distance in the
 *              TLF loop
 *       * [ ] Unwind and carry out the slingshot when done
 *
 * note
 *   Do not allow a full table, resize before it happens
 *     -> hmap_put does not handle inserting on a full table
 *
 * todo
 *   - [ ] Add hmap fitness / resize logic
 *   - [ ] Consider _mm_shuffle_epi8 to execute the swaps for metas and distance
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
size_t hmap_put(struct hmap *const hashmap,
                const size_t key,
                const size_t value)
{
	/* Prepare temp entry */
	const size_t hash    = reduce_fibo(key, hashmap->hash_shift);
	const size_t home    = hash_index(hash);
	const meta_byte meta = hash_meta(hash);

	/* Find given key or first empty slot */
	size_t candidate = hmap_find_or_empty(hashmap, key, home, meta);

	//----------------------------------------------------- empty slot found
	if (is_empty(hashmap->buckets.metas[candidate])) {
		/**
		 * Thierry La Fronde method : Slingshot the rich !
		 *
		 * note
		 *   initial distance write in candidate bucket is necessary for the
		 *   loop to work
		 *
		 * todo
		 *   [ ] Look for ways to to avoid initial write
		 *   [ ] Plan all slingshots, unroll per array
		 */
		// size_t slingshots[PROBE_LENGTH * 2] = {0};
		// int count                           = 0;
		// if (candidate != home) {
		// hashmap->buckets.distances[candidate] = candidate - home;
		for (size_t bucket = candidate; bucket != home; bucket--) {
			if (hashmap->buckets.distances[bucket] <=
			    hashmap->buckets.distances[bucket - 1]) {
				slingshot(hashmap, candidate, bucket);

				// slingshots[count]     = candidate;
				// slingshots[count + 1] = bucket;
				// count += 2;
				// hashmap->buckets.distances[candidate] =
				//     hashmap->buckets.distances[bucket] + candidate - bucket;
				// printf("count = %d | %lu : %lu\n",
				//        count,
				//        slingshots[count - 2],
				//        slingshots[count - 1]);
				// printf(
				//     "distance bucket:%hhu | bucket-1:%hhu -> Slingshot
				//     %lu to
				//     "
				//     "%lu\n",
				//     hashmap->buckets.distances[bucket],
				//     hashmap->buckets.distances[bucket - 1],
				//     bucket,
				//     candidate);
				// dump_hashmap(hashmap);
				candidate = bucket;
// slingshots[count] = bucket;
// count++;
#ifdef DEBUG_HMAP
				hashmap->stats.swap_count++;
#endif /* DEBUG_HMAP */
			}
		}
		// if (count > 0) {

		// 	for (int i = count; i > 0; i -= 2) {
		// 		hashmap->buckets.metas[(slingshots[i - 2])] =
		// 		    hashmap->buckets.metas[(slingshots[i-1])];
		// 		// printf("metas i = %d | %lu : %lu\n",
		// 		//        i,
		// 		//        slingshots[i - 1],
		// 		//        slingshots[i]);
		// 	}
		// 	for (int i = count; i > 0; i -= 2) {
		// 		hashmap->buckets.entries[(slingshots[i - 2])] =
		// 		    hashmap->buckets.entries[(slingshots[i-1])];
		// 		// printf("entries i = %d | %lu : %lu\n",
		// 		//        i,
		// 		//        slingshots[i - 1],
		// 		//        slingshots[i]);
		// 	}
		// 	// puts("--------------------------------------------");
		// }

		// }

		/**
		* Add entry to the store
		*
		* todo
		*   - [ ] Split to its own function
		*   - [ ] Copy to the store properly
		*/
		hashmap->store[hashmap->top].key   = key;
		hashmap->store[hashmap->top].value = value;
#ifdef DEBUG_HMAP
		printf("stored : [%s, %s]\n",
		       hashmap->store[hashmap->top].key,
		       (char *)hashmap->store[hashmap->top].value);
#endif  /* DEBUG_HMAP */
		/**
		 * note
		 *   this says implicitely that current implementation cannot deal with
		 *   chains over 256
		 */
		// meta_byte distance   = candidate - home;
		clobber_bucket_with(
		    hashmap, candidate, meta, candidate - home, hashmap->top);
		// printf(
		//     "home : %lu distance:%lu -> Clobber to "
		//     "%lu\n",
		// 	home,
		//     candidate - home,
		//     candidate);
		hashmap->top++;
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
		hashmap->store[(hashmap->buckets.entries[candidate])].value = value;
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
 * Mark metadata associated to given entry as META_EMPTY
 * Update given hmap stats
 *   -> removed	entry index
 */
static inline void empty_entry(struct hmap *const hashmap, const size_t entry)
{
	hashmap->buckets.metas[entry] = META_EMPTY;
	// hashmap->buckets.distances[entry] = 0;
	hashmap->count--;
}
//----------------------------------------------------------------- Function ---
/**
 * Free given hmap_entry
 *   -> nothing
 *
 * note
 *   This should probably not be used with the store.
 *   Just mark the entry as empty.
 *   Free the store sequentially when deleting the hmap.
 *   Keep destroy_entry only to specifically wipe an entry from the store.
 *
 * todo
 *   - [x] Study how to properly keep track of empty slots in the store if
 *         a hmap_delete is implemented
 */
static inline void destroy_entry(struct hmap *const hashmap, const size_t entry)
{
	// // Use XFREE macro instead of destructor while hmap is a WIP
	// XFREE(hashmap->store[(hashmap->buckets.entries[entry])].key,
	//       "destroy_entry");
	// XFREE(hashmap->store[(hashmap->buckets.entries[entry])].value,
	//       "destroy_entry");

	/**
	 * Rewind top
	 * Find top store entry corresponding bucket entry
	 * Move top store entry to destroyed store entry slot
	 * Wipe top
	 */
	const size_t store_slot = hashmap->buckets.entries[entry];
	hashmap->top--;

	if ((hashmap->top > 0) && (hashmap->top != store_slot)) {
		const size_t top_key = hashmap->store[hashmap->top].key;
#ifdef DEBUG_HMAP
		printf("moving top key : %s to store slot %lu\n", top_key, store_slot);
#endif /* DEBUG_HMAP */

		size_t top_bucket = hmap_find(hashmap, top_key);
#ifdef DEBUG_HMAP
		printf("top_bucket : %lu\n", top_bucket);
#endif /* DEBUG_HMAP */

		hashmap->buckets.entries[top_bucket] = hashmap->buckets.entries[entry];

		hashmap->store[(hashmap->buckets.entries[entry])].key =
		    hashmap->store[hashmap->top].key;
		hashmap->store[(hashmap->buckets.entries[entry])].value =
		    hashmap->store[hashmap->top].value;
		// hashmap->store[hashmap->top].key   = 0; // NULL;
		// hashmap->store[hashmap->top].value = 0; // NULL;
	}
	// empty_entry(hashmap, entry);

	return;
}

//----------------------------------------------------------------- Function ---
/**
 * Look for given key in given hmap
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
size_t hmap_remove(struct hmap *const hashmap, const size_t key)
{
	const size_t entry = hmap_find(hashmap, key);

	/* Given key exists */
	if (entry < hashmap->capacity) {
		/* update store */
		destroy_entry(hashmap, entry);

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
		match_mask = probe_pattern(0, hashmap->buckets.distances + stop_bucket);

		/* is there at least 1 slingshot job ? */
		if (match_mask != 0) {
			/* slingshot entry backward, up to stop bucket */
			stop_bucket += _bit_scan_forward(match_mask);

			/* process buckets.distances while it is hot */
			for (size_t bucket = entry; bucket < stop_bucket; bucket++) {
				hashmap->buckets.distances[bucket] =
				    hashmap->buckets.distances[bucket + 1] - 1;
			}
			/* mark entry distance in last shifted bucket as @home or empty */
			hashmap->buckets.distances[stop_bucket - 1] = 0;

			/* process buckets.entries */
			for (size_t bucket = entry; bucket < stop_bucket; bucket++) {
				hashmap->buckets.entries[bucket] =
				    hashmap->buckets.entries[bucket + 1];
			}

			/* process buckets.metas */
			for (size_t bucket = entry; bucket < stop_bucket; bucket++) {
				hashmap->buckets.metas[bucket] =
				    hashmap->buckets.metas[bucket + 1];
			}
		}
		/* mark entry in last shifted bucket as empty */
		hashmap->buckets.metas[stop_bucket - 1] = META_EMPTY;
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
	/* first rewind 1 entry from the top */
	// while (hashmap->top) {
	// 	hashmap->top--;
	// 	XFREE(hashmap->store[(hashmap->top)].key, "delete_hashmap");
	// 	XFREE(hashmap->store[(hashmap->top)].value, "delete_hashmap");
	// }

	XFREE(hashmap->buckets.metas, "delete_hashmap");
	// XFREE(hashmap->buckets.distances, "delete_hashmap");
	// XFREE(hashmap->buckets.entries, "delete_hashmap");
	// XFREE(hashmap->store, "delete_hashmap");
	XFREE(hashmap, "delete_hashmap");
}

//----------------------------------------------------------------- Function ---
/**
 * Create a new hmap of capacity equal to 2^(given n) + PROBE_LENGTH
 *
 *   Enforce n >= 1
 *   Allocate a Hashmap
 *     If allocation fails,  clean up and abort
 *     -> NULL
 *   Allocate Hashmap buckets
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
 *     + [x] Store n in debug stats
 *   - [ ] Consider one single big alloc
 *     + [ ] Consider setting pointers inside the struct accordingly
 *           after the one big alloc
 *   - [ ] Clear confusion around n / shift amount and rename n
 */

#define MALLOC_HMAP_ARRAY(_array, _capacity)                                   \
	do {                                                                       \
		_array =                                                               \
		    XMALLOC(sizeof(*(_array)) * _capacity, "new_hashmap", #_array);    \
		if (_array == NULL) {                                                  \
			goto err_free_arrays;                                              \
		}                                                                      \
	} while (0)

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

#ifdef DEBUG_HMAP
	struct hmap_stats stats_init = {0, SIZE_MAX, 0, 0, 0, 0, 0, 0, 0, n};
	struct hmap hashmap_init     = {
	    {NULL}, NULL, 0, hash_shift, capacity, 0, stats_init};
#else
	struct hmap hashmap_init = {{NULL}, NULL, 0, hash_shift, capacity, 0};
#endif /* DEBUG_HMAP */

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

	// printf(
	//     "metas_size     : %lu\n"
	//     "distances_size : %lu\n"
	//     "entries_size   : %lu\n",
	//     metas_size,
	//     distances_size,
	//     entries_size);

	// MALLOC_HMAP_ARRAY(new_hashmap->buckets.metas, capacity);
	// MALLOC_HMAP_ARRAY(new_hashmap->buckets.distances, capacity);
	// MALLOC_HMAP_ARRAY(new_hashmap->buckets.entries, capacity);

	char *const pool =
	    XMALLOC(metas_size + distances_size + entries_size + store_size,
	            "new_hashmap",
	            "pool");

	if (pool == NULL) {
		goto err_free_arrays;
	}

	new_hashmap->buckets.metas     = (meta_byte *)pool;
	new_hashmap->buckets.distances = (meta_byte *)(pool + metas_size);
	new_hashmap->buckets.entries =
	    (size_t *)(pool + metas_size + distances_size);
	new_hashmap->store = (struct hmap_entry *)(pool + metas_size +
	                                           distances_size + entries_size);

	// printf(
	//     "buckets.metas     : %p\n"
	//     "buckets.distances : %p\n"
	//     "buckets.entries   : %p\n",
	//     (void *)new_hashmap->buckets.metas,
	//     (void *)new_hashmap->buckets.distances,
	//     (void *)new_hashmap->buckets.entries);

	/* The value is passed as an int, but the function fills the block of
	 * memory using the unsigned char conversion of this value */
	memset(new_hashmap->buckets.metas, META_EMPTY, metas_size);
	//    sizeof(*(new_hashmap->buckets.metas)) * capacity);
	/**
	 * Because distances are initialized to 0
	 * and set to 0 when removing an entry
	 * Probing distances for 0 yields "stop buckets"
	 * aka @home entry or empty bucket
	 */
	memset(new_hashmap->buckets.distances, 0, distances_size);
	//    sizeof(*(new_hashmap->buckets.metas)) * capacity);
	/* Is this enough to be able to check if ptr == NULL ? */
	memset(new_hashmap->buckets.entries, 0, entries_size);
	//    sizeof(*(new_hashmap->buckets.entries)) * capacity);

	//------------------------------------------------------- store init
	/**
	 * todo
	 *   - [ ] Figure out what was the rationale for allocing the store with
	 *         capacity size instead of actual_capacity size before
	 *         actual_capacity was tossed
	 */
	// MALLOC_HMAP_ARRAY(new_hashmap->store, capacity);
	// new_hashmap->top = new_hashmap->store;

	return new_hashmap;

/* free(NULL) is ok, correct ? */
err_free_arrays:
	// XFREE(new_hashmap->buckets.metas, "hmap_new err_free_arrays");
	// XFREE(new_hashmap->buckets.distances, "hmap_new err_free_arrays");
	// XFREE(new_hashmap->buckets.entries, "hmap_new err_free_arrays");
	// XFREE(new_hashmap->store, "hmap_new err_free_arrays");
	XFREE(pool, "hmap_new err_free_arrays");
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
 * Print given Hashmap stats
 *   -> nothing
 */
void stats_hashmap(const struct hmap *const hashmap)
{
	printf("EV entries / bucket = %f\n",
	       expected_entries_per_bucket(hashmap->count,
	                                   hashmap->capacity - PROBE_LENGTH));
	printf(
	    "EV collisions = %f \t-> %f%%\n",
	    expected_collisions(hashmap->count, hashmap->capacity - PROBE_LENGTH),
	    expected_collisions(hashmap->count, hashmap->capacity - PROBE_LENGTH) /
	        (double)hashmap->count * 100);

	printf("EV empty = %f \t-> %f%%\n",
	       expected_empty_buckets(hashmap->count,
	                              hashmap->capacity - PROBE_LENGTH),
	       expected_empty_buckets(hashmap->count,
	                              hashmap->capacity - PROBE_LENGTH) /
	           (double)(hashmap->capacity - PROBE_LENGTH) * 100);

	printf("hashmap->stats.n          : %u\n", hashmap->stats.n);
	printf("hashmap->capacity   : %lu / %lu\n",
	       hashmap->capacity - PROBE_LENGTH,
	       hashmap->capacity);
	printf("hashmap->count      : %lu \t-> %f%%\n",
	       hashmap->count,
	       (double)hashmap->count / (double)(hashmap->capacity - PROBE_LENGTH) *
	           100);
	printf("hashmap->stats.collision_count  : %lu \t-> %lf%%\n",
	       hashmap->stats.collision_count,
	       (double)hashmap->stats.collision_count / (double)hashmap->count *
	           100);

	// printf("hashmap->stats.hashes_tally_or  : %lu\n",
	//        hashmap->stats.hashes_tally_or);
	// printf("hashmap->stats.hashes_tally_and : %lu\n",
	//        hashmap->stats.hashes_tally_and);

	// printf("hashmap->stats.swap_count       : %lu\n",
	//        hashmap->stats.swap_count);

	return;
}
#endif /* DEBUG_HMAP */
//----------------------------------------------------------------- Function ---
/**
 * Dump given Hashmap content
 *   -> nothing
 */
void dump_hashmap(const struct hmap *const hashmap)
{
	size_t empty_bucket = 0;
	int max_distance    = 0;

	for (size_t i = 0; i < hashmap->capacity; i++) {
		if (hashmap->buckets.metas[i] == META_EMPTY) {
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

		printf(" %lu[%d] : %lu | %lu\n",
		       hash_index(reduce_fibo(
		           hashmap->store[(hashmap->buckets.entries[i])].key,
		           hashmap->hash_shift)),
		       hashmap->buckets.distances[i],
		       hashmap->store[(hashmap->buckets.entries[i])].key,
		       hashmap->store[(hashmap->buckets.entries[i])].value);
	}

	printf("empty_buckets       : %lu \t-> %f%%\n",
	       empty_bucket,
	       (double)empty_bucket / (double)(hashmap->capacity - PROBE_LENGTH) *
	           100);
	printf("max_distance        : %d\n", max_distance);

#ifdef DEBUG_HMAP
	stats_hashmap(hashmap);
// print_xor_seed(hashmap);
#endif /* DEBUG_HMAP */
}

//----------------------------------------------------------------- Function ---
/**
 * Go through all buckets
 * Access the store
 *   -> nothing
 */
void sum_bucket(const struct hmap *const hashmap)
{
	size_t sum_key   = 0;
	size_t sum_value = 0;

	for (size_t i = 0; i < hashmap->capacity; i++) {
		sum_key += hashmap->store[(hashmap->buckets.entries[i])].key;
		sum_value += hashmap->store[(hashmap->buckets.entries[i])].value;
	}

	printf(FG_BLUE REVERSE
	       "sum_key             : %lu\n"
	       "sum_value           : %lu\n" RESET,
	       sum_key,
	       sum_value);
}

void sum_store(const struct hmap *const hashmap)
{
	size_t sum_key   = 0;
	size_t sum_value = 0;

	/* first rewind 1 entry from the top */
	size_t top = hashmap->top;
	while (top) {
		top--;
		sum_key += hashmap->store[top].key;
		sum_value += hashmap->store[top].value;
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
	puts("todo\n" FG_BRIGHT_RED
	     "\t- [x] Update hmap->top when doing hmap->remove\n"
	     "\t\t+ [ ] Look for simpler ways to update the store !!\n" RESET
	         FG_BRIGHT_YELLOW
	     "\t- [x] Fill and read an array as a baseline\n"
	     "\t- [x] Add a test that does a constant number of find, get, put,\n"
	     "\t\tremove and compare output at different hmap sizes, load "
	     "factor\n" RESET FG_BRIGHT_GREEN
	     "\t- [ ] Consider a Fat Pointer style struct wrapper to allow\n"
	     "\t\tgeneric key, value type handling\n"
	     "\t- [ ] Split hash functions to hash.c\n" RESET
	     "\t- [ ] Refactor Slingshot sequences by array\n"
	     "\t\t+ [ ] Slingshot ALL buckets.metas then\n"
	     "\t\t+ [ ] Slingshot ALL buckets.distances then\n"
	     "\t\t+ [ ] Slingshot ALL buckets.entries\n"
	     "\t- [x] Implement backward shift deletion\n"
	     "\t- [ ] Profile core table operations\n"
	     "\t\t+ [ ] Isolate them by using fixed size keys, a innocuous hash "
	     "func\n"
	     "\t- [ ] Check boundaries when doing slingshots\n"
	     "\t- [x] Consider tossing actual_capacity \n"
	     "\t\t+ [ ] Hunt potential off-by-1 errors checking entry vs capacity\n"
	     "\t- [ ] Implement baseline non-SIMD linear probing\n"
	     "\t\t+ [ ] Benchmark against SIMD wip versions\n"
	     "\t- [x] Implement the most basic put operation to mock tables\n"
	     "\t- [ ] Try mapping and storing primitive type/values\n"
	     "\t\t+ [ ] Benchmark the difference with store of pointers\n");

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

	// struct hmap *const hashmap = hmap_new(n, NULL);
	struct hmap *const hashmap = hmap_new(n);

	fputs(FG_BLUE REVERSE "Enter desired load factor ? " RESET, stdout);
	scanf("%f", &load_factor);

	//---------------------------------------------------- hmap on the stack
	/* 1 << 18 will require just shy of 7MB on the stack */
	// #define HMAP_SIZE (1 << 8)
	// 	struct hmap_buckets buckets = {(meta_byte[HMAP_SIZE]){0},
	// 	                               (meta_byte[HMAP_SIZE]){0},
	// 	                               (size_t[HMAP_SIZE]){0}};
	// 	struct hmap_entry entry   = {0, 0};
	// 	struct hmap hashmap_stack = {buckets,
	// 	                             (struct hmap_entry[HMAP_SIZE]){entry},
	// 	                             0,
	// 	                             64 - 15,
	// 	                             HMAP_SIZE + PROBE_LENGTH,
	// 	                             0};

	// 	memset(hashmap_stack.buckets.metas, META_EMPTY, HMAP_SIZE);
	// 	struct hmap *const hashmap = &hashmap_stack;
	//------------------------------------------------------------ benchmark
	// setup
	// #ifdef BENCHMARK
	float start, stop, diff, bench_time;
// #endif /* BENCHMARK */

//-------------------------------------------------------------------- setup
// #define KEYPOOL_SIZE 32
// char random_keys[KEYPOOL_SIZE] = {'\0'};
// size_t test_count = (1 << n); // 1 << (n - 1);
#define TEST_COUNT 1000
	size_t load_count = (1 << n) * load_factor;
	printf("load_factor = %f\n", load_factor);
	printf("load_count  = %lu\n", load_count);
	size_t capacity  = hashmap->capacity;
	size_t sum_value = 0;
	char key[256];
	// char *dummy_key   = NULL;
	// char *dummy_value = NULL;

	// srand(__rdtsc());
	printf(FG_BRIGHT_YELLOW REVERSE "Filling hashmap with %lu entries\n" RESET,
	       load_count);

	printf(FG_BRIGHT_YELLOW REVERSE "hmap->top : %lu\n" RESET, hashmap->top);

	START_BENCH(start);
	for (size_t k = 0; k < load_count; k++) {
		hmap_put(hashmap, k, k);
	}

	printf(FG_BRIGHT_YELLOW REVERSE "Done !\n" RESET);
	printf(FG_YELLOW REVERSE "hmap->top : %lu\n" RESET, hashmap->top);
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
				// printf("\tk = %lu\n", k);
				sum_value += hmap_get(hashmap, k);
			}
			printf("sum : %lu\n", sum_value);

			for (int k = TEST_COUNT - 1; k >= 0; k--) {
				// printf("\tk = %d\n", k);
				sum_value -= hmap_get(hashmap, k);
			}
			printf("sum : %lu\n", sum_value);

			for (size_t k = TEST_COUNT; k > 0; k--) {
				// printf("\tk = %lu\n", k);
				size_t random_key = rand() % capacity;
				// printf("random_key : %lu\n", random_key);
				sum_value += hmap_get(hashmap, random_key);
				// printf("found : %lu\n", hmap_get(hashmap, random_key));
			}
			printf("sum : %lu\n", sum_value);

			continue;
		}
		//-------------------------------------------------- findrand
		if ((strcmp(key, "findrand")) == 0) {
			sum_value = 0;

			for (size_t k = TEST_COUNT - 1; k > 0; k--) {
				// printf("\tk = %lu\n", k);
				size_t random_key = rand() % capacity;
				// printf("random_key : %lu\n", random_key);
				sum_value += hmap_get(hashmap, random_key);
				// printf("found : %lu\n", hmap_get(hashmap, random_key));
			}
			printf("sum : %lu\n", sum_value);
			continue;
		}

#ifdef DEBUG_HMAP
		// if ((strcmp(key, "seed")) == 0) {
		// 	print_xor_seed(hashmap);
		// 	continue;
		// }
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
			// START_BENCH(start);
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
				// return EXIT_FAILURE;
			}
			// Abort if seed is only junk or prefixed with junk
			// Ignore junk after valid seed though
			// Note : a leading 0 means base 8, thus 09 will yield 0 as 9
			//        is considered junk in base 8
			if (key_end == key) {
				printf("Error : could NOT process given seed : junk !\n");
				// return EXIT_FAILURE;
			}

			size_t result = hmap_find(hashmap, numeric_key);

			if (result > hashmap->capacity) {
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
			// STOP_BENCH(start, stop, diff, bench_time);
		}
	}

	//-------------------------------------------------------------- cleanup
	hmap_delete_hashmap(hashmap);
	return 0;
}