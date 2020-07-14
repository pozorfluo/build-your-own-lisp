//------------------------------------------------------------------ SUMMARY ---
/**
 * Based on the work of the Abseil Team and publications of Daniel Lemire,
 * Peter Kankowski, Malte Skarupke, Maurice Herlihy, Nir Shavit, Moran Tzafrir,
 * Emmanuel Goossaert.
 *
 * Implement a hash map tuned for lispy / LispEnv
 */

#include <errno.h>
// #include <inttypes.h> /* strtoumax */
#include <limits.h> /* UINT_MAX */
#include <stddef.h> /* size_t */
#include <stdint.h> /* uint32_t, uint64_t */
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* memcpy */

#include "ansi_esc.h"
#include "debug_xmalloc.h"
#include "hfunc.h"
//------------------------------------------------------------ CONFIGURATION ---
#include "configuration.h"
//------------------------------------------------------------ MAGIC NUMBERS ---
#define HMAP_NOT_FOUND SIZE_MAX
#define HMAP_PROBE_LENGTH 32
//------------------------------------------------------------------- MACROS ---

//------------------------------------------------------------- DECLARATIONS ---
typedef signed char meta_byte;

enum meta_ctrl {
	META_EMPTY    = -128, /* 0b10000000 */
	META_OCCUPIED = 0,    /* 0b0xxxxxxx */
	/* Think of META_OCCUPIED as anything like 0b0xxxxxxx */
	/* aka 0 <= META_OCCUPIED < 128 */
};

/**
 * todo Rework this for a more generic setup when you are more comfortable using
 *      unions.
 */
struct hmap_entry {
	// size_t key;
	// char *key; /* string key stored elsewhere */
	char key[HMAP_INLINE_KEY_SIZE]; /* string key stored inline are not null
	                                   terminated */
	size_t value;
};

struct hmap_bucket {
	meta_byte meta;     /* 1 byte per bucket */
	meta_byte distance; /* 1 byte per bucket */
	size_t entry;       /* sizeof(size_t) bytes per bucket */
};

/**
 * Check packing https://godbolt.org/z/dvdKqM
 */
struct hmap {
	struct hmap_bucket *buckets;
	struct hmap_entry *store;
	size_t top;
	// size_t key_size;
	size_t hash_shift; /* shift amount necessary for desired hash depth */
	size_t capacity;   /* actual capacity */
	size_t count;      /* occupied entries count */
};

//----------------------------------------------------- FORWARD DECLARATIONS ---
static inline size_t hash_index(const size_t hash)
    __attribute__((const, always_inline));

static inline meta_byte hash_meta(const size_t hash)
    __attribute__((const, always_inline));

static inline void destroy_entry(struct hmap *const hashmap, const size_t entry)
    __attribute__((always_inline));

size_t hmap_find(const struct hmap *const hashmap, const char *const key)
    __attribute__((pure));

static inline size_t hmap_find_or_empty(const struct hmap *const hashmap,
                                        const char *const key,
                                        size_t index,
                                        const meta_byte meta)
    __attribute__((pure, always_inline));

void dump_hashmap(const struct hmap *const hashmap);

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
size_t hmap_find(const struct hmap *const hm, const char *const key)
{
	size_t hash    = HREDUCE(HFUNC(key), hm->hash_shift);
	size_t index   = hash_index(hash);
	meta_byte meta = hash_meta(hash);

	do {
		if (hm->buckets[index].meta == meta) {
			// if (hm->store[(hm->buckets[index].entry)].key == key) {
			if (HCMP(hm->store[(hm->buckets[index].entry)].key, key
			         //  ,HMAP_INLINE_KEY_SIZE
			         ) == 0) {
				/* Found key ! */
				return index;
			}
		}
		else if (hm->buckets[index].meta == META_EMPTY) {
			/* if there is any empty slot*/
			/* no need to check the next */
			/* -> key does NOT exist */
			break;
		}

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
static inline size_t hmap_find_or_empty(const struct hmap *const hm,
                                        const char *const key,
                                        size_t index,
                                        const meta_byte meta)
{
	do {
		if (hm->buckets[index].meta == meta) {
			if (HCMP(hm->store[(hm->buckets[index].entry)].key, key
			         //  ,HMAP_INLINE_KEY_SIZE
			         ) == 0) {
				/* Found key ! */
				return index;
			}
		}
		else if (hm->buckets[index].meta == META_EMPTY) {
			/* if there is any empty slot*/
			/* no need to check the next */
			/* -> key does NOT exist */
			return index;
		}

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
	const size_t entry = hmap_find(hm, key);
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
size_t hmap_put(struct hmap *const hm,
                const char *const key,
                const size_t value)
{
	/* Prepare temp entry */
	const size_t hash    = reduce_fibo(HFUNC(key), hm->hash_shift);
	const size_t home    = hash_index(hash);
	const meta_byte meta = hash_meta(hash);

	/* Find given key or first empty slot */
	size_t candidate = hmap_find_or_empty(hm, key, home, meta);

	//----------------------------------------------------- empty slot found
	if (is_empty(hm->buckets[candidate].meta)) {
		/* Thierry La Fronde method : Slingshot the rich ! */
		for (size_t bucket = candidate; bucket != home; bucket--) {
			hm->buckets[candidate].distance = candidate - home;

			if (hm->buckets[bucket].distance <=
			    hm->buckets[bucket - 1].distance) {

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

		// hm->store[hm->top].key   = key;
		// strncpy(hm->store[hm->top].key, key, HMAP_INLINE_KEY_SIZE);
		memcpy(hm->store[hm->top].key, key, strlen(key));
		hm->store[hm->top].value = value;
		hm->top++;
		hm->count++;
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
static inline void empty_entry(struct hmap *const hm, const size_t entry)
{
	hm->buckets[entry].meta = META_EMPTY;
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
	const size_t store_slot = hm->buckets[entry].entry;
	hm->top--;

	if ((hm->top > 0) && (hm->top != store_slot)) {
		const char *const top_key = hm->store[hm->top].key;

		const size_t top_bucket = hmap_find(hm, top_key);

		hm->buckets[top_bucket].entry = hm->buckets[entry].entry;
		hm->store[(hm->buckets[entry].entry)] = hm->store[hm->top];
		// memcpy(hm->store[(hm->buckets[entry].entry)].key,
		//        top_key,
		//        HMAP_INLINE_KEY_SIZE);
		// // hm->store[(hm->buckets[entry].entry)].key   = hm->store[hm->top].key;
		// hm->store[(hm->buckets[entry].entry)].value = hm->store[hm->top].value;
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
size_t hmap_remove(struct hmap *const hm, const char *const key)
{
	const size_t entry = hmap_find(hm, key);

	/* Given key exists */
	if (entry != HMAP_NOT_FOUND) {
		/* update store */
		destroy_entry(hm, entry);
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

		while (hm->buckets[stop_bucket].distance != 0) {
			stop_bucket++;
		}

		for (size_t bucket = entry; bucket < stop_bucket; bucket++) {
			hm->buckets[bucket].meta     = hm->buckets[bucket + 1].meta;
			hm->buckets[bucket].distance = hm->buckets[bucket + 1].distance - 1;
			hm->buckets[bucket].entry    = hm->buckets[bucket + 1].entry;
		}
		/* mark entry distance in last shifted bucket as @home or empty */
		hm->buckets[stop_bucket - 1].distance = 0;

		/* mark entry in last shifted bucket as empty */
		hm->buckets[stop_bucket - 1].meta = META_EMPTY;

		// /* process buckets.entries */
		// for (size_t bucket = entry; bucket < stop_bucket; bucket++) {
		// 	hm->buckets[bucket].entry = hm->buckets[bucket + 1].entry;
		// }

		// /* process buckets.metas */
		// for (size_t bucket = entry; bucket < stop_bucket; bucket++) {
		// 	hm->buckets[bucket].meta = hm->buckets[bucket + 1].meta;
		// }
	}

	return entry;
}

//----------------------------------------------------------------- Function ---
/**
 * Free given Hashmap
 *   -> nothing
 */
void hmap_delete_hashmap(struct hmap *const hashmap)
{
	/* hashmap->buckets is the head of the single alloc pool */
	XFREE(hashmap->buckets, "hmap_delete_hashmap");
	XFREE(hashmap, "delete_hashmap");
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
struct hmap *hmap_new(const size_t n)
{
	//---------------------------------------------- advertised capacity
	/* tab_hash requires a mininum of 8 bits of hash space */
	/* including the 7 bits required for meta_byte */
	size_t capacity = (n < 1) ? (1u << 1) : (1u << n);
	// const size_t hash_depth = capacity << 7;

	//-------------------------------------------------------- hmap init
	/* actual capacity */
	capacity += HMAP_PROBE_LENGTH;

	/**
	 * shift amount necessary for desired hash depth including the 7 bits
	 * required for meta_byte with reduce function
	 *
	 * note
	 *   this assumes __WORDSIZE == 64
	 */
	const size_t hash_shift = 64 - 7 - n;

	struct hmap hashmap_init = {NULL, NULL, 0, hash_shift, capacity, 0};

	struct hmap *const new_hashmap =
	    XMALLOC(sizeof(struct hmap), "new_hashmap", "new_hashmap");
	memcpy(new_hashmap, &hashmap_init, sizeof(struct hmap));

	if (new_hashmap == NULL) {
		goto err_free_hashmap;
	}

	//----------------------------------------------------- buckets init
	const size_t buckets_size = sizeof(*(new_hashmap->buckets)) * capacity;
	const size_t store_size   = sizeof(*(new_hashmap->store)) * capacity;

	char *const pool =
	    XMALLOC(buckets_size + store_size, "new_hashmap", "pool");

	if (pool == NULL) {
		goto err_free_pool;
	}

	new_hashmap->buckets = (struct hmap_bucket *)pool;
	new_hashmap->store   = (struct hmap_entry *)(pool + buckets_size);

	/* The value is passed as an int, but the function fills the block of
	 * memory using the unsigned char conversion of this value */
	memset(new_hashmap->buckets, 0, buckets_size);

	for (size_t i = 0; i < capacity; i++) {
		new_hashmap->buckets[i].meta = META_EMPTY;
		// printf("init bucket[%lu] = {%02hhd, %02hhd, %lu}\n",
		//        i,
		//        new_hashmap->buckets[i].meta,
		//        new_hashmap->buckets[i].distance,
		//        new_hashmap->buckets[i].entry);
	}
	/**
	 * Because distances are initialized to 0
	 * and set to 0 when removing an entry
	 * Probing distances for 0 yields "stop buckets"
	 * aka @home entry or empty bucket
	 */
	/* Is this enough to be able to check if ptr == NULL ? */

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
		if (hm->buckets[i].meta == META_EMPTY) {
			empty_bucket++;
			printf("\x1b[100mhashmap->\x1b[30mbucket[%lu]>> EMPTY <<\x1b[0m\n",
			       i);
			continue;
		}

		/* Color code distance from home using ANSI esc code values */
		int colour;

		switch (hm->buckets[i].distance) {
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
		max_distance = (hm->buckets[i].distance > max_distance)
		                   ? hm->buckets[i].distance
		                   : max_distance;
		printf("\x1b[10%dmhashmap->\x1b[30mbucket[%lu]\x1b[0m>>", colour, i);

		printf(
		    " %lu[%d] : %.16s | %lu\n",
		    hash_index(reduce_fibo(HFUNC(hm->store[(hm->buckets[i].entry)].key),
		                           hm->hash_shift)),
		    hm->buckets[i].distance,
		    hm->store[(hm->buckets[i].entry)].key,
		    hm->store[(hm->buckets[i].entry)].value);
	}

	printf("empty_buckets       : %lu \t-> %f%%\n",
	       empty_bucket,
	       (double)empty_bucket / (double)(hm->capacity - HMAP_PROBE_LENGTH) *
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
		// sum_key += hm->store[(hm->buckets[i].entry)].key;
		sum_value += hm->store[(hm->buckets[i].entry)].value;
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
		// sum_key += hashmap->store[top].key;
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
	puts(WELCOME_MESSAGE);

#ifdef __AVX__
	puts("__AVX__ 1");
	printf("HMAP_PROBE_LENGTH %d\n", HMAP_PROBE_LENGTH);
#endif /* __AVX__ */

	printf("__WORDSIZE %d\n", __WORDSIZE);
	printf("RAND_MAX  %d\n", RAND_MAX);
	printf("SIZE_MAX  %lu\n", SIZE_MAX);
	printf("UINT_MAX  %u\n", UINT_MAX);
	printf("size_t %lu bytes\n", sizeof(size_t));
	printf("struct hmap        %lu bytes\n", sizeof(struct hmap));
	printf("struct hmap_bucket %lu bytes\n", sizeof(struct hmap_bucket));
	printf("struct hmap_entry  %lu bytes\n", sizeof(struct hmap_entry));
	printf("struct meta_byte   %lu bytes\n", sizeof(meta_byte));
	// printf("HFUNC  %s\n", HFUNC);
	// printf("HREDUCE  %s\n", HREDUCE);
	// printf("HCMP  %s\n", HCMP);
	// printf("HCOPY  %s\n", HCOPY);

	// uint32_t seed = 31;
	size_t n = 8;
	float load_factor;
	int unused_result __attribute__((unused));
	char *unused_result_s __attribute__((unused));

	fputs(FG_BRIGHT_BLUE REVERSE
	      "Table size is 2^n. Enter n ( default n = 8 ) ? " RESET,
	      stdout);
	unused_result = scanf("%lu", &n);

	struct hmap *const hashmap = hmap_new(n);

	fputs(FG_BLUE REVERSE "Enter desired load factor ? " RESET, stdout);
	unused_result = scanf("%f", &load_factor);

	//---------------------------------------------------------------- setup
	SETUP_BENCH(repl);
	size_t load_count = (1 << n) * load_factor;
	printf("load_factor = %f\n", load_factor);
	printf("load_count  = %lu\n", load_count);
	// size_t capacity  = hashmap->capacity;
	size_t sum_value = 0;
	char key[256];
	char random_key[HMAP_INLINE_KEY_SIZE] = {'\0'};

//------------------------------------------------- random keys pool
#define KEYPOOL_SIZE 4096
	char random_keys[KEYPOOL_SIZE] = {'\0'};
	int r;

	for (size_t i = 0; i < KEYPOOL_SIZE; i++) {
		r = rand() % 32;
		// Replace with a-z for interval [0,25]
		// Leave '\0' for interval [26,32]
		if (r < 26) {
			random_keys[i] = (char)(r + 0x61);
			// putchar(random_keys[i]);
		}
	}

	char random_notfound_keys[KEYPOOL_SIZE] = {'\0'};

	for (size_t i = 0; i < KEYPOOL_SIZE; i++) {
		r = rand() % 32;
		// Replace with a-z for interval [0,25]
		// Leave '\0' for interval [26,32]
		if (r < 26) {
			random_notfound_keys[i] = (char)(r + 0x41);
			// putchar(random_notfound_keys[i]);
		}
	}

	printf(FG_BRIGHT_YELLOW REVERSE "Filling hashmap with %lu entries\n" RESET,
	       load_count);

	printf(FG_BRIGHT_YELLOW REVERSE "hmap->top : %lu\n" RESET, hashmap->top);

	START_BENCH(repl);
	for (size_t k = 0; k < load_count; k++) {
		for (size_t i = 0; i < HMAP_INLINE_KEY_SIZE; i++) {
			random_key[i] = (char)(rand() % 26 + 0x61);
		}
		hmap_put(hashmap, random_key, k);
		// hmap_put(hashmap, &random_keys[rand() % KEYPOOL_SIZE], k);
	}

	printf(FG_BRIGHT_YELLOW REVERSE "Done !\n" RESET);
	printf(FG_YELLOW REVERSE "hmap->top : %lu\n" RESET, hashmap->top);
	unused_result_s = fgets(key, 255, stdin);
	//----------------------------------------------------------- input loop
	for (;;) {
		STOP_BENCH(repl);
		fputs("\x1b[102m > \x1b[0m", stdout);
		unused_result_s = fgets(key, 255, stdin);
		size_t length   = strlen(key);

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
			// for (size_t k = 0; k < load_count; k++) {
			size_t is_stuck = hashmap->top;
			while (hashmap->top) {
				printf(FG_BRIGHT_YELLOW REVERSE
				       "hmap->top : %lu"
				       " : %.*s" 
				       " : %.lu" 
				       " : %.lu\n" 
					   RESET,
				       hashmap->top,
					   HMAP_INLINE_KEY_SIZE,
				       hashmap->store[hashmap->top - 1].key,
				       hashmap->store[hashmap->top - 1].value,
					   hmap_find(hashmap, hashmap->store[hashmap->top - 1].key));
				hmap_remove(hashmap, hashmap->store[hashmap->top - 1].key);
				if(hashmap->top == is_stuck) { break;}
				is_stuck = hashmap->top;
			}
			printf(FG_BRIGHT_YELLOW REVERSE "hmap->top : %lu\n" RESET,
			       hashmap->top);
			continue;
		}
		//-------------------------------------------------- fill
		if ((strcmp(key, "fill")) == 0) {
			for (size_t k = 0; k < load_count; k++) {
				for (size_t i = 0; i < HMAP_INLINE_KEY_SIZE; i++) {
					random_key[i] = (char)(rand() % 26 + 0x61);
				}
				hmap_put(hashmap, random_key, k);
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

		//-------------------------------------------------- findin
		if ((strcmp(key, "findin")) == 0) {
			sum_value = 0;

			for (size_t k = 0; k < TEST_COUNT; k++) {
				sum_value +=
				    hmap_get(hashmap, hashmap->store[k % hashmap->count].key);
			}

			printf("sum : %lu\nTEST_COUNT : %d\n", sum_value, TEST_COUNT);
			continue;
		}
		//-------------------------------------------------- findrandin
		if ((strcmp(key, "findrandin")) == 0) {
			sum_value = 0;

			for (size_t k = 0; k < TEST_COUNT; k++) {
				sum_value += hmap_get(
				    hashmap, hashmap->store[rand() % hashmap->count].key);
			}

			printf("sum : %lu\nTEST_COUNT : %d\n", sum_value, TEST_COUNT);
			continue;
		}
		//-------------------------------------------------- findnot
		if ((strcmp(key, "findnot")) == 0) {
			sum_value = 0;
			for (size_t k = 0; k < TEST_COUNT; k++) {
				/* todo Check that % KEYPOOL_SIZE doesn't yield OOB indexes */
				sum_value += hmap_get(
				    hashmap, &random_notfound_keys[rand() % KEYPOOL_SIZE]);
			}
			printf("sum : %lu\nTEST_COUNT : %d\n", sum_value, TEST_COUNT);
			continue;
		}
		//-------------------------------------------------- findrand
		if ((strcmp(key, "findrand")) == 0) {
			sum_value = 0;
			for (size_t k = 0; k < TEST_COUNT; k++) {
				/* todo Check that % KEYPOOL_SIZE doesn't yield OOB indexes */
				sum_value +=
				    hmap_get(hashmap, &random_keys[rand() % KEYPOOL_SIZE]);
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
			size_t result = hmap_find(hashmap, key);

			if (result == HMAP_NOT_FOUND) {
				puts("Key not found ! \n");
			}
			else {
				printf("Looking for %.16s -> found @ %lu\n", key, result);
				printf("\t\t-> value : %lu\n", hmap_get(hashmap, key));
				printf("Removing entry !\n");
				hmap_remove(hashmap, key);
				printf(FG_YELLOW REVERSE "hmap->top : %lu\n" RESET,
				       hashmap->top);
			}
		}
	}

	//-------------------------------------------------------------- cleanup
	hmap_delete_hashmap(hashmap);
	return 0;
}