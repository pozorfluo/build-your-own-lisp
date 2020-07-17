//------------------------------------------------------------------ SUMMARY ---
/**
 * Based on the work of the Abseil Team and publications of Daniel Lemire,
 * Peter Kankowski, Malte Skarupke, Maurice Herlihy, Nir Shavit, Moran Tzafrir,
 * Emmanuel Goossaert.
 *
 * Implement a hash map tuned for lispy / LispEnv
 */

#include <errno.h>
#include <stdint.h> /* uint32_t, uint64_t */
#include <stdio.h>
#include <stdlib.h> /* malloc */
#include <string.h> /* memcpy, strlen, strnlen */

#include "ansi_esc.h"
#include "debug_xmalloc.h"
#include "hfunc.h"
#include "hmap.h"

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
 * Return next higher power of 2.
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
static inline size_t hash_index(const size_t hash) { return hash >> 7; }
//----------------------------------------------------------------- Function ---
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
static inline int compare_fixed128_keys(const char *const key_a,
                                        const char *const key_b)
{
	return !((*(uint64_t *)key_a) && (*(uint64_t *)key_b) &&
	         (*(uint64_t *)key_a + 8) && (*(uint64_t *)key_b + 8));
}

//----------------------------------------------------------------- Function ---
size_t hmap_find(const struct hmap *const hm, const char *const key)
{
	// /* truncate or pad with \0 given key to HMAP_INLINE_KEY_SIZE */
	// size_t key_size = strnlen(key, HMAP_INLINE_KEY_SIZE);
	// if (key_size != HMAP_INLINE_KEY_SIZE) {
	// 	char padded_key[HMAP_INLINE_KEY_SIZE] = {'\0'};
	// 	char *dest                            = &padded_key;
	// 	char *src                             = key;
	// 	while (key_size--) {
	// 		*dest++ = *src++;
	// 	}
	// 	key = &padded_key;
	// }

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
 * todo
 *   - [ ] Investigate ways to probe for Match or META_EMPTY at once
 *   - [X] Consider using SIZE_MAX directly as an error
 *     + [ ] Add exists() inlinable function
 */
static inline size_t find_n(const struct hmap *const hm,
                            const char *const key,
                            const size_t length)
{
	size_t hash    = HREDUCE(HFUNC(key, length), hm->hash_shift);
	size_t index   = hash_index(hash);
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
			// if (hm->store[(hm->buckets[index].entry)].key == key) {
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
 * todo
 *   - [ ] Add hmap fitness / resize trigger logic
 */
size_t hmap_put(struct hmap *const hm,
                const char *const key,
                const size_t value)
{
	/* Prepare temp entry */
	const size_t key_size = strnlen(key, HMAP_INLINE_KEY_SIZE);
	const size_t hash     = HREDUCE(HFUNC(key, key_size), hm->hash_shift);
	const size_t home     = hash_index(hash);
	const meta_byte meta  = hash_meta(hash);

	/* Find given key or first empty slot */
	size_t candidate = find_or_empty(hm, key, home, meta);

	//----------------------------------------------------- empty slot found
	if (is_empty(hm->buckets[candidate].meta)) {
		/* Thierry La Fronde method : Slingshot the rich ! */
		for (size_t bucket = candidate; bucket != home; bucket--) {
			hm->buckets[candidate].distance = candidate - home;

			if (hm->buckets[bucket].distance <=
			    hm->buckets[bucket - 1].distance) {

				// printf("slingshot[ %lu -> %lu ]\n", candidate, bucket);
				hm->buckets[candidate].distance =
				    hm->buckets[bucket].distance + candidate - bucket;

				hm->buckets[candidate].meta = hm->buckets[bucket].meta;

				hm->buckets[candidate].entry = hm->buckets[bucket].entry;
				candidate                    = bucket;
			}
		}

		hm->buckets[candidate].meta     = meta;
		hm->buckets[candidate].distance = candidate - home;
		hm->buckets[candidate].entry    = hm->top;

		/* Same as strncpy but not caring about result being null terminated */
		// size_t key_size = strnlen(key, HMAP_INLINE_KEY_SIZE);
		char *dest    = hm->store[hm->top].key;
		size_t length = HMAP_INLINE_KEY_SIZE;

		while (length--) {
			// printf("%lu\n", length);
			*dest++ = '\0';
		}
		memcpy(hm->store[hm->top].key, key, key_size);

		hm->store[hm->top].value = value;
		hm->top++;

		printf(FG_BRIGHT_GREEN REVERSE
		       " hash         %lu \n" RESET
		       " hash reduced %lu \n"
		       " home         %lu \n"
		       " candidate    %lu \n"
		       " meta         %lu \n"
		       " entry        %lu \n"
		       " entry ^ hash %lu \n" ,
		       HFUNC(key, key_size),
		       HREDUCE(HFUNC(key, key_size), hm->hash_shift),
		       home,
		       candidate,
		       (size_t)meta,
		       hm->buckets[candidate].entry,
		       HFUNC(key, key_size) ^ hm->buckets[candidate].entry);
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
	const size_t store_slot = hm->buckets[entry].entry;
	hm->top--;

	if ((hm->top > 0) && (hm->top != store_slot)) {
		const char *const top_key = hm->store[hm->top].key;
		const size_t top_bucket =
		    find_n(hm, top_key, strnlen(top_key, HMAP_INLINE_KEY_SIZE));

		printf(FG_BRIGHT_CYAN REVERSE
		       "top key    : %s \n"
		       "top bucket : %lu \n" RESET,
		       top_key,
		       top_bucket);

		hm->buckets[top_bucket].entry = hm->buckets[entry].entry;
		// hm->buckets[top_bucket]               = hm->buckets[entry];
		hm->store[(hm->buckets[entry].entry)] = hm->store[hm->top];
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

		for (size_t bucket = entry; bucket < stop_bucket; bucket++) {
			hm->buckets[bucket] = hm->buckets[bucket + 1];
			hm->buckets[bucket].distance--;

			// printf("backwardshiftdel[ %lu -> %lu ]\n", bucket + 1, bucket);
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
 * Does not clear the store.
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
 */
void hmap_free(struct hmap *const hm)
{
	/* hashmap->buckets is the head of the single alloc pool */
	XFREE(hm->buckets, "hmap_delete_hashmap");
	XFREE(hm, "delete_hashmap");
}

//----------------------------------------------------------------- Function ---
/**
 * Create a new hmap of capacity equal to 2^(given n) + HMAP_PROBE_LENGTH
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
struct hmap *hmap_new(const size_t requested_capacity)
{
	/* tab_hash requires a mininum of 8 bits of hash space */
	/* including the 7 bits required for meta_byte */
	// size_t capacity = (n < 1) ? (1u << 1) : (1u << n);

	/* map actual capacity */
	size_t map_capacity =
	    next_pow2(requested_capacity / HMAP_MAX_LOAD) + HMAP_PROBE_LENGTH;

	/**
	 * shift amount necessary for desired hash depth including the 7 bits
	 * required for meta_byte with reduce function
	 *
	 * todo
	 *   - [ ] Look for a portable __builtin_clzll alternative
	 */
	const size_t hash_shift =
	    HWIDTH - 7 - ((HWIDTH - 1) - __builtin_clzll(map_capacity));

	struct hmap *const new_hm =
	    XMALLOC(sizeof(struct hmap), "new_hm", "new_hm");

	if (new_hm == NULL) {
		goto err_free_hashmap;
	}

	const size_t buckets_size = sizeof(*(new_hm->buckets)) * map_capacity;
	const size_t store_size =
	    sizeof(*(new_hm->store)) * (requested_capacity + 1);

	char *const pool = XMALLOC(buckets_size + store_size, "new_hm", "pool");

	if (pool == NULL) {
		goto err_free_pool;
	}

	struct hmap init_hm = {.buckets = (struct hmap_bucket *)pool,
	                       .store = (struct hmap_entry *)(pool + buckets_size),
	                       .top   = 0,
	                       .hash_shift = hash_shift,
	                       .capacity   = map_capacity};
	*new_hm = init_hm;

	/* XMALLOC is calling calloc / takes cares of setting mem to 0 */
	// memset(new_hm->buckets, 0, buckets_size);

	for (size_t i = 0; i < map_capacity; i++) {
		new_hm->buckets[i].meta = META_EMPTY;
	}

	printf(FG_BRIGHT_CYAN REVERSE " store_size : %lu \n" RESET,
	       (size_t)(requested_capacity + 1));
	printf(FG_CYAN REVERSE " capacity : %lu \n" RESET, (size_t)(map_capacity));
	printf(FG_BLUE REVERSE " hash_shift : %lu \n" RESET, (size_t)(hash_shift));
	/**
	 * Because distances are initialized to 0
	 * and set to 0 when removing an entry
	 * Probing distances for 0 yields "stop buckets"
	 * aka @home entry or empty bucket
	 */
	/* Is this enough to be able to check if ptr == NULL ? */

	//------------------------------------------------------- store init
	return new_hm;

/* free(NULL) is ok, correct ? */
err_free_pool:
	XFREE(pool, "hmap_new err_free_arrays");
err_free_hashmap:
	XFREE(new_hm, "new_hm");
	return NULL;
}
