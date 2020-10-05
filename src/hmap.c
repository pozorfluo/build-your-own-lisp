//------------------------------------------------------------------ SUMMARY ---
/**
 * Based on the work of the Abseil Team and publications of Daniel Lemire,
 * Peter Kankowski, Malte Skarupke, Maurice Herlihy, Nir Shavit, Moran Tzafrir,
 * Emmanuel Goossaert.
 *
 * Implement a hash map tuned for lispy / LispEnv
 */

#include <errno.h>
#include <stddef.h> /* offsetof, size_t */
#include <stdint.h> /* uint32_t, uint64_t */
#include <stdio.h>
#include <stdlib.h> /* malloc */
#include <string.h> /* memcpy, strlen, strnlen */

#include "ansi_esc.h"
#include "debug_xmalloc.h"
#include "hfunc.h"
#include "hmap.h"
#include "hmap_test.h"

//----------------------------------------------------- FORWARD DECLARATIONS ---
static inline size_t hash_index(const size_t hash)
    __attribute__((const, always_inline));

static inline meta_byte hash_meta(const size_t hash)
    __attribute__((const, always_inline));

static inline void destroy_entry(struct hmap *const hashmap, const size_t entry)
    __attribute__((always_inline));

static inline size_t find_n(const struct hmap *const hm,
                            const char *const key,
                            const size_t length) __attribute__((pure));
static inline size_t find_or_empty(const struct hmap *const hashmap,
                                   const char *const key,
                                   size_t index,
                                   const meta_byte meta)
    __attribute__((pure, always_inline));

static inline size_t next_pow2(size_t v) __attribute__((const, always_inline));
//----------------------------------------------------------------- Function ---
/**
 * Return next higher power of 2 for given unsigned integer.
 *
 * see https://graphics.stanford.edu/~seander/bithacks.html
 */
static inline size_t next_pow2(size_t v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
#ifdef HWIDTH_64
	v |= v >> 32;
#endif
	return ++v;
}
//----------------------------------------------------------------- Function ---
static inline size_t hash_index(const size_t hash) { return hash >> 15; }
//----------------------------------------------------------------- Function ---
static inline meta_byte hash_meta(const size_t hash) { return hash & 0x7FFF; }
//----------------------------------------------------------------- Function ---
/**
 * Check if bucket at given index in given hashmap is empty
 *   -> Truth value of predicate
 */
static inline int is_empty(const meta_byte meta)
{
	return (meta == META_EMPTY);
}
//----------------------------------------------------------------- Function ---
static inline int is_bucket_empty(const struct hmap *const hashmap,
                                  const size_t index)
{
	return is_empty(hashmap->buckets[index].meta);
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
//----------------------------------------------------------------- Function ---
static inline int is_bucket_occupied(const struct hmap *const hashmap,
                                     const size_t index)
{
	return is_occupied(hashmap->buckets[index].meta);
}
//----------------------------------------------------------------- Function ---
/**
 * Compare given fixed size 16 bytes string keys
 *     If bytes comparison matches over length
 *       -> 0
 *     Else
 *       -> 1
 */
// static inline int compare_fixed128_keys(const char *const key_a,
//                                         const char *const key_b)
// {
// 	return !((*(uint64_t *)key_a) && (*(uint64_t *)key_b) &&
// 	         (*(uint64_t *)key_a + 8) && (*(uint64_t *)key_b + 8));
// }

//----------------------------------------------------------------- Function ---
size_t hmap_find(const struct hmap *const hm, const char *const key)
{
	return find_n(hm, key, strnlen(key, HMAP_INLINE_KEY_SIZE));
}

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
 * @todo
 *   - [ ] Investigate ways to probe for Match or META_EMPTY at once
 *   - [X] Consider using SIZE_MAX directly as an error
 *     + [ ] Add exists() inlinable function
 * @todo Consider short circuiting probe on META_EMPTY or @home
 */
static inline size_t find_n(const struct hmap *const hm,
                            const char *const key,
                            const size_t length)
{
	size_t hash = HREDUCE(HFUNC(key, length), hm->hash_shift);
	// size_t hash = HFUNC(key, length);
	size_t index = hash_index(hash);
	// size_t index = hash_index(HREDUCE(hash, hm->hash_shift));
	// size_t index   = HREDUCE(hash, hm->hash_shift);
	meta_byte meta = hash_meta(hash);

	// printf(FG_MAGENTA REVERSE
	//        "looking for : %*.*s | hash[%lu] index[%lu] m[%d]\n" RESET,
	//        HMAP_INLINE_KEY_SIZE,
	//        HMAP_INLINE_KEY_SIZE,
	//        key,
	//        hash,
	//        index,
	//        meta);

	// putchar('\n');
	do {
		if (hm->buckets[index].meta == meta) {
			// putchar('.');
			if (HCMP(hm->store[(hm->buckets[index].entry)].key,
			         key,
			         HMAP_INLINE_KEY_SIZE) == 0) {
				// putchar('!');
				/* Found key ! */
				return index;
			}
		}
		else if (hm->buckets[index].meta == META_EMPTY) {
			/* if there is any empty slot*/
			/* no need to check the next */
			/* -> key does NOT exist */
			// putchar('o');
			break;
		}
		// putchar('x');
		index++;
	} while (index < hm->capacity);

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
 * note
 *   Because distances are initialized to 0 and set to 0 when removing an entry,
 *   probing distances for 0 yields "stop buckets" aka @home entry or empty
 *   bucket.
 *
 * todo
 *   - [ ] Investigate ways to probe for Match or META_EMPTY at once
 */
static inline size_t find_or_empty(const struct hmap *const hm,
                                   const char *const key,
                                   size_t index,
                                   const meta_byte meta)
{
	// putchar('\n');
	do {
		if (hm->buckets[index].meta == meta) {
			// putchar('.');
			if (HCMP(hm->store[(hm->buckets[index].entry)].key,
			         key,
			         HMAP_INLINE_KEY_SIZE) == 0) {
				// putchar('!');
				/* Found key ! */
				return index;
			}
		}
		else if (hm->buckets[index].meta == META_EMPTY) {
			/* if there is any empty slot*/
			/* no need to check the next */
			/* -> key does NOT exist */
			// putchar('o');
			return index;
		}
		// putchar('x');
		index++;
	} while (index < hm->capacity);

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
size_t hmap_get(const struct hmap *const hm, const char *const key)
{
	// const size_t entry = hmap_find(hm, key);
	const size_t entry = find_n(hm, key, strnlen(key, HMAP_INLINE_KEY_SIZE));
	// void *value        = NULL;
	size_t value = 0;

	if (entry != HMAP_NOT_FOUND) {
		value = hm->store[(hm->buckets[entry].entry)].value;
	}

	return value;
}
//----------------------------------------------------------------- Function ---
/**
 * Rehash given Hashmap's store to given map.
 */
static inline void rehash(struct hmap *const hm, struct hmap_bucket *const map)
{
	size_t store_index = hm->top;

	while (store_index--) {
		const char *const key = hm->store[store_index].key;
		// printf("rehashing store[%lu] : %s \n", store_index, key);
		/* Prepare temp entry */
		size_t key_size      = strnlen(key, HMAP_INLINE_KEY_SIZE);
		const size_t hash    = HREDUCE(HFUNC(key, key_size), hm->hash_shift);
		const size_t home    = hash_index(hash);
		const meta_byte meta = hash_meta(hash);
		// const size_t hash    = HFUNC(key, key_size);
		// const size_t home    = HREDUCE(hash, hm->hash_shift);
		// const meta_byte meta = hash_meta(hash);

		size_t candidate = home;

		// printf(FG_MAGENTA REVERSE
		//        "looking for : %*.*s | hash[%lu] index[%lu] m[%d]\n" RESET,
		//        HMAP_INLINE_KEY_SIZE,
		//        HMAP_INLINE_KEY_SIZE,
		//        key,
		//        hash,
		//        home,
		//        meta);
		do {
			if (map[candidate].meta == META_EMPTY) {
				// printf("candidate[%lu]\n", candidate);
				break;
			}
			// printf(".");
			candidate++;
		} while (candidate < hm->capacity);

		/* Thierry La Fronde method : Slingshot the rich ! */
		map[candidate].distance = candidate - home;
		for (size_t bucket = candidate; bucket != home; bucket--) {

			int bucket_distance        = map[bucket].distance;
			int candidate_distance     = candidate - home;
			int candidate_bucket_delta = candidate - bucket;

			if (bucket_distance <= map[bucket - 1].distance &&
			    bucket_distance + candidate_bucket_delta <=
			        candidate_distance) {

				// printf("slingshot[ %lu -> %lu ]\n", candidate, bucket);
				map[candidate].distance =
				    map[bucket].distance + candidate_bucket_delta;

				map[candidate].meta  = map[bucket].meta;
				map[candidate].entry = map[bucket].entry;
				candidate            = bucket;
			}
		}

		map[candidate].meta     = meta;
		map[candidate].distance = candidate - home;
		map[candidate].entry    = store_index;
	}
}

//----------------------------------------------------------------- Function ---
/**
 * Grow given Hashmap's store or map or both as necessary.
 *
 * If any allocation fails, clean up and abort
 *     -> NULL
 * Else
 *     -> pointer to grown hmap
 */
static inline struct hmap *grow(struct hmap *const hm)
{
	struct hmap_entry *grown_store;
	struct hmap_bucket *grown_map;
	/**
	 * Check if its worth it to realloc the store and keep the map.
	 *
	 * note
	 *   Tune this carefully, realloc may have to make a copy of the whole
	 *   store and it is probably not worth triggering this too often for small
	 *   size bumps.
	 *
	 *   This only possibly affects the first 'grow' event, after that, size
	 *   of store and map are 'synched'.
	 */
	const size_t max_store_capacity =
	    (hm->capacity - HMAP_PROBE_LENGTH) * HMAP_MAX_LOAD;

	if (hm->store_capacity * HMAP_STORE_MUST_GROW <
	    max_store_capacity) { // <= 0) { //
		// printf(FG_BRIGHT_MAGENTA REVERSE
		//        " Growing the store from [%lu] to [%lu] \n" RESET,
		//        hm->store_capacity,
		//        max_store_capacity);
		// printf(FG_MAGENTA REVERSE " hm->store     -> %p \n" RESET,
		//        (void *)hm->store);

		grown_store =
		    realloc(hm->store, sizeof(*(hm->store)) * (max_store_capacity + 1));
		if (grown_store == NULL) {
			goto err_free_grown_store;
		}

		memset(grown_store + hm->top,
		       0,
		       sizeof(*(hm->store)) * (max_store_capacity + 1 - hm->top));

		hm->store = grown_store;

		// printf(FG_MAGENTA REVERSE " hm->store     -> %p \n" RESET,
		//        (void *)hm->store);

		// printf(FG_CYAN REVERSE " Grown the store from [%lu] to [%lu] \n"
		// RESET,
		//        hm->store_capacity,
		//        max_store_capacity);
		hm->store_capacity = max_store_capacity;
	}
	else {
		/**
		 * Create a new double size map,
		 * Grow the store,
		 * Rehash the store to new map,
		 * Free old map.
		 *
		 * note
		 *   Old capacity was extended by HMAP_PROBE_LENGTH, we do not need the
		 *   extension to be doubled.
		 */

		size_t new_capacity       = hm->capacity * 2 - HMAP_PROBE_LENGTH;
		const size_t new_map_size = sizeof(*grown_map) * new_capacity;

		// printf(FG_BRIGHT_MAGENTA REVERSE
		//        " Growing the map from [%lu] to [%lu] : %lu bytes \n" RESET,
		//        hm->capacity,
		//        new_capacity,
		//        new_map_size);

		grown_map = XMALLOC(new_map_size, "grow", "buckets");
		if (grown_map == NULL) {
			goto err_free_grown_map;
		}
		// printf(FG_MAGENTA REVERSE " grown_map     -> %p \n" RESET,
		//        (void *)grown_map);

		for (size_t i = 0; i < new_capacity; i++) {
			grown_map[i].meta = META_EMPTY;
		}
		/**
		 * todo Rewind updating Hashmap stats on failure.
		 */
		// const size_t new_shift =
		//     HWIDTH - 7 - ((HWIDTH - 1) - __builtin_clzll(new_capacity));
		// hm->hash_shift =
		//     HWIDTH - 7 - ((HWIDTH - 1) - __builtin_clzll(new_capacity));
		hm->hash_shift--;
		hm->capacity = new_capacity;

		const size_t new_store_capacity = (hm->store_capacity * 2);
		const size_t new_store_size =
		    sizeof(*(hm->store)) * (new_store_capacity + 1);

		// printf(FG_BRIGHT_MAGENTA REVERSE
		//        " Growing the store from [%lu] to [%lu] : %lu bytes \n" RESET,
		//        hm->store_capacity,
		//        (hm->store_capacity * 2),
		//        new_store_size);
		// printf(FG_MAGENTA REVERSE " hm->store     -> %p \n" RESET,
		//        (void *)hm->store);

		grown_store = realloc(hm->store, new_store_size);
		if (grown_store == NULL) {
			goto err_free_grown_store;
		}

		memset(grown_store + hm->top,
		       0,
		       sizeof(*(hm->store)) * (new_store_capacity + 1 - hm->top));

		// if (grown_store != hm->store) {
		// 	printf(FG_MAGENTA REVERSE " moving to     -> %p \n" RESET,
		// 	       (void *)grown_store);
		// 	/* UB */
		// 	printf(FG_MAGENTA REVERSE "               -> %ld \n" RESET,
		// 	       (long)grown_store - (long)hm->store);
		// }
		hm->store          = grown_store;
		hm->store_capacity = new_store_capacity;
		// printf(FG_MAGENTA REVERSE " hm->store     -> %p \n" RESET,
		//        (void *)hm->store);
		// printf(FG_MAGENTA REVERSE " hm->buckets   -> %p \n" RESET,
		//        (void *)hm->buckets);

		rehash(hm, grown_map);
		XFREE(hm->buckets, "grow free old map");
		hm->buckets = grown_map;
	}

	return hm;

err_free_grown_map:
	XFREE(grown_map, "grow err_free_grown_map");
err_free_grown_store:
	XFREE(grown_store, "grow err_free_grown_store");
	return NULL;
}

struct hmap *debug_grow(struct hmap *const hm) { return grow(hm); }
void debug_rehash(struct hmap *const hm) { rehash(hm, hm->buckets); }

//----------------------------------------------------------------- Function ---
/**
 * Insert a new entry in given Hashmap for given key, value pair or update
 * an existing value
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
 *     Increment hmap entry count / store cursor
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
 * 	 Home (bucket - distance to home ) can only increase when traversing
 *   buckets.
 *
 *   Because distances are initialized to 0 and set to 0 when removing an entry,
 *   probing distances for 0 yields "stop buckets" aka @home entry or empty
 *   bucket.
 */
size_t hmap_put(struct hmap *const hm, const char *key, const size_t value)
{
	/* Prepare temp entry */
	size_t key_size   = strnlen(key, HMAP_INLINE_KEY_SIZE);
	const size_t hash = HREDUCE(HFUNC(key, key_size), hm->hash_shift);
	// const size_t hash = HFUNC(key, key_size);
	const size_t home = hash_index(hash);
	// const size_t home = hash_index(HREDUCE(hash, hm->hash_shift));
	// const size_t home    = HREDUCE(hash, hm->hash_shift);
	const meta_byte meta = hash_meta(hash);

	/* Find given key or first empty slot */
	size_t candidate = find_or_empty(hm, key, home, meta);
	// printf(FG_BRIGHT_GREEN REVERSE " home %16lu " RESET FG_BRIGHT_GREEN
	//                                " %-16lu candidate" RESET,
	//        home,
	//        candidate);
	// printf(REVERSE "\t%*.*s | %lu \n" RESET,
	//        HMAP_INLINE_KEY_SIZE,
	//        HMAP_INLINE_KEY_SIZE,
	//        key,
	//        value);
	// printf(FG_BRIGHT_GREEN REVERSE " hash         %lu \n" RESET
	//                                " hash reduced %lu \n"
	//                                " home         %lu \n"
	//                                " candidate    %lu \n"
	//                                " meta         %lu \n"
	//                                " entry        %lu \n",
	//        hash,
	//        HREDUCE(HFUNC(key, key_size), hm->hash_shift),
	//        home,
	//        candidate,
	//        (size_t)meta,
	//        (size_t)hm->buckets[candidate].entry);
	//----------------------------------------------------- empty slot found
	if (is_empty(hm->buckets[candidate].meta)) {
		/* ------------------------------------------------------------------ */
		/**
		 *  Thierry La Fronde method : Slingshot the rich !
		 *
		 * todo
		 *   - [ ] Have another look at the 'blank slingshot' happening on
		 first
		 *         go around. Consider if it is worth avoiding.
		 */
		hm->buckets[candidate].distance = candidate - home;
		// for (size_t bucket = candidate;
		//      bucket != home;
		//      bucket--) {
			for (int bucket = candidate - 1; bucket > (int)home; bucket--) {

			int bucket_distance        = hm->buckets[bucket].distance;
			int candidate_distance     = candidate - home;
			int candidate_bucket_delta = candidate - bucket;

			// if (hm->buckets[bucket].distance <= hm->buckets[bucket -
			// 1].distance) {
			if (bucket_distance <= hm->buckets[bucket - 1].distance &&
			    bucket_distance + candidate_bucket_delta <=
			        candidate_distance) {
				// printf(REVERSE
				//        "candidate %lu[__][" FG_RED "%.2d" FG_DEFAULT
				//        "] | bucket %d <= %d ? slingshot[ %d -> %lu ]\n"
				//        RESET, candidate, hm->buckets[candidate].distance,
				//        hm->buckets[bucket].distance,
				//        hm->buckets[bucket - 1].distance,
				//        bucket,
				//        candidate);
				// dump_hashmap_horizontal(hm, home, 21, bucket, candidate);

				/* This rewrites the same value on 'blank slingshots' */
				hm->buckets[candidate].distance =
				    hm->buckets[bucket].distance + candidate_bucket_delta;

				hm->buckets[candidate].meta  = hm->buckets[bucket].meta;
				hm->buckets[candidate].entry = hm->buckets[bucket].entry;
				candidate                    = bucket;
			}
		}
		hm->buckets[candidate].meta     = meta;
		hm->buckets[candidate].distance = candidate - home;
		hm->buckets[candidate].entry    = hm->top;
		/* ------------------------------------------------------------------ */
		// /* Robin hood method. */
		// struct hmap_bucket tmp;
		// struct hmap_bucket swap_bucket = {
		//     .meta = meta, .distance = 0, .entry = hm->top};

		// candidate = home;
		// /**
		//  * While candidate bucket is NOT EMPTY
		//  *   if candidate bucket distance < swap bucket distance
		//  *     swap buckets
		//  *   swap bucket distance ++
		//  *   candidate bucket ++
		//  *
		//  * note
		//  *   On first swap, swap_bucket will point to an top empty entry with
		//  a
		//  *   default value (0 here).
		//  *
		//  * todo
		//  *   - [ ] Consider that if home is not empty it is NEVER SWAPPED
		//  *         with swap_bucket. Is it worth starting at home + 1 ?
		//  */
		// while (hm->buckets[candidate].meta != META_EMPTY) {

		// 	if (hm->buckets[candidate].distance < swap_bucket.distance) {
		// 		// swap_bucket.entry == hm->top
		// 		//     ? printf(REVERSE "swap_bucket [" FG_YELLOW "__"
		// FG_DEFAULT
		// 		//                      "][" FG_RED "%.2d" FG_DEFAULT "]\n"
		// RESET,
		// 		//              swap_bucket.distance)
		// 		//     : printf(REVERSE "swap_bucket [" FG_YELLOW
		// 		//                      "%.2lu" FG_DEFAULT "][" FG_RED
		// 		//                      "%.2d" FG_DEFAULT "]\n" RESET,
		// 		//              hm->store[swap_bucket.entry].value,

		// 		//              swap_bucket.distance);

		// 		// dump_hashmap_horizontal(
		// 		//     hm, home, 21, candidate, candidate);

		// 		tmp                    = hm->buckets[candidate];
		// 		hm->buckets[candidate] = swap_bucket;
		// 		swap_bucket            = tmp;
		// 	}
		// 	swap_bucket.distance++;
		// 	candidate++;
		// }
		// hm->buckets[candidate] = swap_bucket;
		/* ------------------------------------------------------------------ */

		/* Same as strncpy but not caring about result being null terminated
		 */
		// size_t key_size = strnlen(key, HMAP_INLINE_KEY_SIZE);
		// 		char *dest    = hm->store[hm->top].key;
		// 		size_t length = HMAP_INLINE_KEY_SIZE;

		// 		while (length--) {
		// 			// printf("%lu\n", length);
		// 			*dest++ = '\0';
		// 		}
		// 				dest = hm->store[hm->top].key;
		// #pragma GCC ivdep
		// 		while (key_size--) {
		// 			*dest++ = *key++;
		// 		}
		/**
		 * Compiler is clever enough, no need to flex.
		 * see https://godbolt.org/z/cM948a
		 */
		memset(hm->store[hm->top].key + key_size,
		       '\0',
		       HMAP_INLINE_KEY_SIZE - key_size);
		memcpy(hm->store[hm->top].key, key, key_size);

		hm->store[hm->top].value = value;
		// printf(FG_BRIGHT_WHITE REVERSE "writing to store[%lu] \n" RESET,
		//        hm->top);
		hm->top++;
		if (hm->top > hm->store_capacity) {
			grow(hm);
		}
		// dump_hashmap_horizontal(hm, home, 21, home, candidate);
		// puts(FG_MAGENTA "done\n" RESET);
	}
	//------------------------------------------------------ given key found
	else {
		hm->store[(hm->buckets[candidate].entry)].value = value;
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
// static inline void empty_entry(struct hmap *const hm, const size_t entry)
// {
// 	hm->buckets[entry].meta = META_EMPTY;
// 	hm->top--;
// }
//----------------------------------------------------------------- Function ---
/**
 * Free given hmap entry
 *
 * Rewind top
 * If given entry is not at store top
 *   Find top store entry corresponding bucket entry
 *   Move top store entry to destroyed store entry slot
 * (Wipe top)
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
	const size_t store_slot = hm->buckets[entry].entry;
	const size_t top        = --hm->top;

	if ((top > 0) && (top != store_slot)) {
		const char *const top_key = hm->store[top].key;

		/** @todo - [ ] Consider storing top - 1 bucket index in top somehow
		 *              to obviate that find_n
		 */
		const size_t top_bucket =
		    find_n(hm, top_key, strnlen(top_key, HMAP_INLINE_KEY_SIZE));

		hm->buckets[top_bucket].entry = store_slot;
		// hm->buckets[top_bucket]               = hm->buckets[entry];
		hm->store[store_slot] = hm->store[top];
	}
	memset(hm->store + top, 0, sizeof(*(hm->store)));
	return;
}

// printf(FG_BRIGHT_CYAN REVERSE
//        "top key    : %s \n"
//        "top bucket : %lu \n" RESET,
//        top_key,
//        top_bucket);
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
 * todo
 *   - [ ] Assess if probing at entry + 1 yelds a cache miss when
 *         read/writing to entry
 *     + [ ] Do not bother ! probing at (advertised)capacity + 1 will
 *            try to read 1 bucket out of bound !!
 *     + [ ] Be careful though, you are going to have to ignore entry
 *           distance because it may be @home
 */
size_t hmap_remove(struct hmap *const hm, const char *const key)
{
	const size_t entry = find_n(hm, key, strnlen(key, HMAP_INLINE_KEY_SIZE));

	/* Given key exists */
	if (entry != HMAP_NOT_FOUND) {
		/* update store */
		destroy_entry(hm, entry);

		size_t stop_bucket = entry + 1;

		while (hm->buckets[stop_bucket].distance) {
			stop_bucket++;
		}

		if (stop_bucket != entry + 1) {
			for (size_t bucket = entry; bucket < stop_bucket; bucket++) {
				// printf("backwardshiftdel[ %lu -> %lu ] d %hu -> %hu \n",
				//        bucket + 1,
				//        bucket,
				//        hm->buckets[bucket + 1].distance,
				//        hm->buckets[bucket + 1].distance - 1);
				hm->buckets[bucket] = hm->buckets[bucket + 1];
				hm->buckets[bucket].distance--;
			}
		}
		/* mark entry distance in last shifted bucket as @home or empty */
		hm->buckets[stop_bucket - 1].distance = 0;

		/* mark entry in last shifted bucket as empty */
		hm->buckets[stop_bucket - 1].meta = META_EMPTY;
	}

	return entry;
}

//----------------------------------------------------------------- Function ---
/**
 * Clear given Hashmap entries.
 *
 * Does not clear the store. This should not be used if Hashmap is supposed
 * to manage pointees with their sole pointer in the store.
 *   -> nothing
 */
void hmap_clear(struct hmap *const hm)
{
	size_t capacity           = hm->capacity;
	const size_t buckets_size = sizeof(*(hm->buckets)) * capacity;

	memset(hm->buckets, 0, buckets_size);

	for (size_t i = 0; i < capacity; i++) {
		hm->buckets[i].meta = META_EMPTY;
	}

	hm->top = 0;
}

//----------------------------------------------------------------- Function ---
/**
 * Free given Hashmap
 *   -> nothing
 *
 * @todo Consider a defensive wrapper version,
 *       see https://stackoverflow.com/a/1879597
 *
 */
void hmap_free(struct hmap *hm)
{
	XFREE(hm->buckets, "hmap_free");
	XFREE(hm->store, "hmap_free");
	XFREE(hm, "hmap_free");
}

//----------------------------------------------------------------- Function ---
/**
 * Create a new hmap of given requested capacity.
 *
 * i.e., for a requested capacity of n
 *
 *   advertised map capacity :
 *     next power of 2 of ( n / max load factor)
 *
 *   actual map capacity :
 *     next power of 2 of ( n / max load factor) + probe length
 *
 *   advertised store capacity :
 *     n
 *
 *   actual store capacity :
 *     n + 1
 *
 * note
 *   Advertised map capacity is extended to actual map capacity by the probe
 *   length to obviate some bounds checking.
 *
 *   Hash shift is the shift amount necessary to reduce a full hash to a hash
 *   depth able to address map capacity + 15 bits of secondary hash
 *   stored as metadata.
 *
 *   The 15 bits of secondary hash are used in a 'bloom filter fashion' during
 *   probing to obviate costly keys comparisons.
 *
 *   Derive advertised map capacity from requested capacity
 *   Derive hash shift from advertised map capacity
 *   Derive actual map capacity from advertised map capacity
 *   Allocate a hmap
 *     If allocation fails, clean up and abort
 *       -> NULL
 *   Allocate map buckets
 *     If allocation fails, clean up and abort
 *       -> NULL
 *   Allocate key, value store
 *     If allocation fails, clean up and abort
 *       -> NULL
 *   Init hmap
 *   Init map buckets as being empty
 *     -> pointer to new hmap
 *
 * @todo Consider what happens when size_t is 4 bytes large and a new hmap
 *       is requested with a capacity > (1 >> (32 - 15)). Is having a full hash
 *       size > (32 + 15) enough ?
 */
struct hmap *hmap_new(const size_t requested_capacity)
{
	size_t map_capacity =
	    next_pow2(requested_capacity / HMAP_MAX_LOAD); // + HMAP_PROBE_LENGTH;

	const size_t hash_shift =
	    HWIDTH - 15 - ((HWIDTH - 1) - __builtin_clzll(map_capacity));
	// HWIDTH - ((HWIDTH - 1) - __builtin_clzll(map_capacity));

	map_capacity += HMAP_PROBE_LENGTH;

	// struct hmap *const new_hm =
	struct hmap *new_hm = XMALLOC(sizeof(struct hmap), "new_hm", "new_hm");

	if (new_hm == NULL) {
		goto err_free_hashmap;
	}

	const size_t buckets_size = sizeof(*(new_hm->buckets)) * map_capacity;
	const size_t store_size =
	    sizeof(*(new_hm->store)) * (requested_capacity + 1);

	struct hmap_bucket *buckets = XMALLOC(buckets_size, "new_hm", "buckets");
	if (buckets == NULL) {
		goto err_free_buckets;
	}
	struct hmap_entry *store = XMALLOC(store_size, "new_hm", "store");
	if (store == NULL) {
		goto err_free_store;
	}

	struct hmap init_hm = {.buckets        = buckets,
	                       .store          = store,
	                       .top            = 0,
	                       .hash_shift     = hash_shift,
	                       .capacity       = map_capacity,
	                       .store_capacity = requested_capacity};
	*new_hm             = init_hm;

	/* XMALLOC is calling calloc / takes cares of setting mem to 0 */
	// memset(new_hm->buckets, 0, buckets_size);

	for (size_t i = 0; i < map_capacity; i++) {
		new_hm->buckets[i].meta = META_EMPTY;
	}

	printf(FG_BRIGHT_CYAN REVERSE " store_size : %lu + 1\n" RESET,
	       requested_capacity);
	printf(FG_CYAN REVERSE " capacity : %lu \n" RESET, (size_t)(map_capacity));
	printf(FG_BLUE REVERSE " hash_shift : %lu \n" RESET, (size_t)(hash_shift));
	printf(FG_BRIGHT_RED REVERSE
	       " hmap                  -> %p \n"
	       " buckets               -> %p \n"
	       " store                 -> %p \n"
	       " buckets offset         : %lu \n"
	       " store   offset         : %lu \n"
	       " pool                   : %lu bytes \n"
	       " buckets                : %lu bytes \n"
	       " store                  : %lu bytes \n"
	       " buckets alignment ctrl : %lu bytes \n"
	       " store   alignment ctrl : %lu bytes \n",
	       (void *)new_hm,
	       (void *)new_hm->buckets,
	       (void *)new_hm->store,
	       offsetof(struct hmap, buckets),
	       offsetof(struct hmap, store),
	       buckets_size + store_size,
	       buckets_size,
	       store_size,
	       buckets_size % (__WORDSIZE / 8),
	       store_size % (__WORDSIZE / 8));

	return new_hm;

err_free_store:
	XFREE(store, "hmap_new err_free_store");
err_free_buckets:
	XFREE(buckets, "hmap_new err_free_buckets");
err_free_hashmap:
	XFREE(new_hm, "new_hm");
	return NULL;
}
