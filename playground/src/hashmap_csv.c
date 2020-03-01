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

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint32_t, uint64_t */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash_murmur3_nose.h"

//------------------------------------------------------------ MAGIC NUMBERS ---
#define TABLE_SIZE 8191

//------------------------------------------------------------------- MACROS ---
#define ARRAY_LENGTH(_array) (sizeof(_array) / sizeof((_array)[0]))

#define BEFORE(_xor_link, _after) xor_link(_xor_link, _after)
#define AFTER(_xor_link, _before) xor_link(_xor_link, _before)

//----------------------------------------------------- FORWARD DECLARATIONS ---
// struct HashmapEntry;
// struct Hashmap;
// typedef struct HashmapEntry HashmapEntry;
// typedef struct Hashmap Hashmap;

static inline size_t mod_2n1(uint_fast64_t dividend, const unsigned int n)
    __attribute__((const, always_inline));
static size_t mod_hash128(const Hash128 hash, const unsigned int n)
    __attribute__((const, always_inline));

//------------------------------------------------------------- DECLARATIONS ---
/**
 * todo
 *   - [ ] Consider a wrapper Hash type to deal with different hash bit depth
 */
typedef Hash128 (*HashFunc)(const void *, const size_t, const uint32_t);
typedef void (*ValueDestructor)(void *);

typedef struct HashmapEntry {
	Hash128 hash;
	char *key;
	void *value;
	// - [ ] Consider replacing destructor with
	//       dispatch(HashmapEntry->type)
	ValueDestructor destructor;
	struct HashmapEntry *next_in_bucket; /* bucket list*/
	struct HashmapEntry *xor_link;       /* insertion list */
} HashmapEntry;

typedef struct Hashmap {
	const uint32_t seed;
	HashFunc function;
	unsigned int n;
	size_t capacity;
	size_t count;
	size_t collisions;
	HashmapEntry **buckets;
	HashmapEntry *head; /* insertion list */
	HashmapEntry *tail; /* insertion list */
} Hashmap;

//----------------------------------------------------------------- Function ---
/**
 * Compute the xor_link pointer of given previous, next pointer pair
 *   -> XORed pointer to HashmapEntry
 */
static inline HashmapEntry *xor_link(HashmapEntry *previous, HashmapEntry *next)
{
	return (HashmapEntry *)((uintptr_t)(previous) ^ (uintptr_t)(next));
}

//----------------------------------------------------------------- Function ---
/**
 * Insert given HashmapEntry node after given target HashmapEntry node
 *   -> XORed pointer of inserted node
 */
// static inline HashmapEntry *insert_after(HashmapEntry *target,
//                                          HashmapEntry *inserted)
// {

// 	return inserted->xor_link;
// }
//----------------------------------------------------------------- Function ---
/**
 * Insert given HashmapEntry node at given Hashmap tail
 *   -> XORed pointer of inserted node
 */
static inline HashmapEntry *insert_xor_list(Hashmap *hashmap,
                                            HashmapEntry *node)
{
	// todo
	//   - [ ] Init tail so that the NULL check is superfluous
	if (hashmap->tail == NULL) {
		hashmap->head  = node;
		node->xor_link = xor_link(hashmap->head, NULL);
	}
	else {
		HashmapEntry *before    = BEFORE(hashmap->tail->xor_link, NULL);
		hashmap->tail->xor_link = xor_link(before, node);
	}
	node->xor_link = xor_link(hashmap->tail, NULL);
	hashmap->tail  = node;
	return node->xor_link;
}

//----------------------------------------------------------------- Function ---
/**
 * Compute modulus division by 2^n - 1 for given integer dividend
 *   -> Remainder
 */
static inline size_t mod_2n1(uint_fast64_t dividend, const unsigned int n)
{
	const uint_fast64_t divisor = (1u << n) - 1;
	uint64_t remainder;

	for (remainder = dividend; dividend > divisor; dividend = remainder) {
		for (remainder = 0; dividend; dividend >>= n) {
			remainder += dividend & divisor;
		}
	}
	return remainder == divisor ? 0 : remainder;
}

//----------------------------------------------------------------- Function ---
/**
 * Compute modulus division by 2^n - 1 for given Hash128
 *   -> Remainder
 */
static inline size_t mod_hash128(const Hash128 hash, const unsigned int n)
{
	const uint32_t h1 = (hash.hi & 0xFFFFFFFF00000000ul) >> 32;
	const uint32_t h2 = (hash.hi & 0x00000000FFFFFFFFul);
	const uint32_t h3 = (hash.lo & 0xFFFFFFFF00000000ul) >> 32;
	const uint32_t h4 = (hash.lo & 0x00000000FFFFFFFFul);

	return mod_2n1(
	    (mod_2n1((mod_2n1((mod_2n1(h1, n) << 32) + h2, n) << 32) + h3, n)
	     << 32) +
	        h4,
	    n);
}

//----------------------------------------------------------------- Function ---
/**
 * Create a new HashmapEntry for given key, value pair
 * Store given ValueDestructor, handle value agnostically
 * Store given hash, trade time for space when resizing Hashmap
 *   -> pointer to new HashmapEntry
 *
 * todo
 *   - [x] Figure out how bad it is to let caller allocate for value and
 *         merely point to it
 *     + [ ] Consider making a proper copy of value
 *     + [ ] See
 * https://stackoverflow.com/questions/33721361/how-to-get-copy-of-void-pointer-data
 */
HashmapEntry *new_hashmap_entry(const char *key,
                                // const void *value,
                                void *alloced_value,
                                ValueDestructor destructor,
                                const Hash128 hash)
{
	HashmapEntry *new_entry;
	new_entry = malloc(sizeof(HashmapEntry));

	new_entry->key = malloc(strlen(key) + 1); // + sizeof('\0')
	strcpy(new_entry->key, key);

	new_entry->value          = alloced_value;
	new_entry->destructor     = destructor;
	new_entry->hash           = hash;
	new_entry->next_in_bucket = NULL;
	new_entry->xor_link       = NULL;
	return new_entry;
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
 *     Append entry to insertion list
 * 		-> nothing
 *   Else traverse entry list in bucket
 *     If key found
 *       Delete value
 *       Replace with copy of given value
 *         -> nothing
 *   Create new entry
 *   Append entry to entry list in bucket
 *   Append entry to insertion list
 *   Increment Hashmap entry count ( Check Hashmap fitness )
 *     -> nothing
 *
 *  todo
 *   - [ ] Split insertion list logic to separate functions
 */
void put_hashmap(Hashmap *hashmap,
                 const char *key,
                 void *alloced_value,
                 ValueDestructor destructor)
{
	const Hash128 hash = hashmap->function(key, strlen(key), hashmap->seed);
	const size_t index = mod_hash128(hash, hashmap->n);

	// HashmapEntry *entry = hashmap->buckets[index];

	/* Empty bucket */
	if (hashmap->buckets[index] == NULL) {
		hashmap->buckets[index] =
		    new_hashmap_entry(key, alloced_value, destructor, hash);
		/* Append to  insertion list */
		insert_xor_list(hashmap, hashmap->buckets[index]);
		// check_hashmap(hashmap);
		hashmap->count++;
		return;
	}

	/* Collision */
	HashmapEntry *entry    = hashmap->buckets[index];
	HashmapEntry *previous = NULL;

	// printf("\n !!! About to resolve collision !!! \n");
	do {
		if (strcmp(entry->key, key) == 0) {
			entry->destructor(entry->value);
			entry->value = alloced_value;
			// printf("!!! Done resolving collision -> Key Exist !!! \n");

			return;
		}
		previous = entry;
		entry    = entry->next_in_bucket;
	} while (entry != NULL);

	previous->next_in_bucket =
	    new_hashmap_entry(key, alloced_value, destructor, hash);
	insert_xor_list(hashmap, hashmap->buckets[index]);
	// check_hashmap(hashmap);
	hashmap->count++;
	hashmap->collisions++;
	// printf("!!! Done resolving collision -> Key appended to bucket list
	// !!!\n");
	return;
}

//----------------------------------------------------------------- Function ---
/**
 * Free given HashmapEntry
 *   -> nothing
 */
static inline void destroy_entry(HashmapEntry *entry)
{
	free(entry->key);
	entry->destructor(entry->value);
	free(entry);
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
	for (size_t i = 0; i < hashmap->capacity; i++) {
		HashmapEntry *entry = hashmap->buckets[i];

		if (entry == NULL) {
			continue;
		}

		HashmapEntry *next;
		while (entry != NULL) {
			next = entry->next_in_bucket;
			// printf("delete %s\n", entry->key);
			destroy_entry(entry);
			entry = next;
		}
	}
	free(hashmap->buckets);
	free(hashmap);
}

//----------------------------------------------------------------- Function ---
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
	for (size_t i = 0; i < hashmap->capacity; i++) {
		HashmapEntry *entry = hashmap->buckets[i];

		if (entry == NULL) {
			continue;
		}
		printf("\x1b[102mhashmap->\x1b[30mbucket[%lu]\x1b[0m\n", i);

		for (;;) {
			printf("\t %016lx %016lx : %s\n",
			       entry->hash.hi,
			       entry->hash.lo,
			       entry->key);
			if (entry->next_in_bucket == NULL) {
				break;
			}
			entry = entry->next_in_bucket;
		}
		// putchar('\n');
	}
	printf("hashmap->seed       : %u\n", hashmap->seed);
	printf("hashmap->function   : %p\n", (unsigned char *)&(hashmap->function));
	printf("hashmap->n          : %u\n", hashmap->n);
	printf("hashmap->capacity   : %lu\n", hashmap->capacity);
	printf("hashmap->count      : %lu\n", hashmap->count);
	printf("hashmap->collisions : %lu\n", hashmap->collisions);
	printf("hashmap->buckets    : %p\n", (void *)hashmap->buckets);
	printf("hashmap->head       : %p\n", (void *)hashmap->head);
	printf("hashmap->tail       : %p\n", (void *)hashmap->tail);
}

//----------------------------------------------------------------- Function ---
/**
 * Print given Hashmap content
 *   -> nothing
 */
void print_hashmap(Hashmap *hashmap)
{
	HashmapEntry *entry = hashmap->head;
	// HashmapEntry *previous = NULL;
	// HashmapEntry *next;
	printf("head -> %32s : %s\n", entry->key, (char *)entry->value);
	printf("tail -> %32s : %s\n",
	       hashmap->tail->key,
	       (char *)hashmap->tail->value);
	// todo
	//   - [ ] fix traversing insertion list in print_hashmap

	// while (entry != NULL) {
	// 	printf("%32s : %s\n", entry->key, (char *)entry->value);
	// 	next     = AFTER(entry->xor_link, previous);
	// 	previous = entry;
	// 	entry    = next;
	// }
	putchar('\n');
}

//----------------------------------------------------------------- Function ---
/**
 * Create Hashmap of capacity equal to 2^(given n) - 1,  with given seed
 *
 *   Init a dummy
 *   Allocate a Hashmap
 *   Copy dummy to Hashmap
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
                     const uint32_t seed,
                     HashFunc function)
{
	const size_t capacity      = (1u << n) - 1;
	const Hashmap hashmap_init = {
	    seed, function, n, capacity, 0, 0, NULL, NULL, NULL};

	Hashmap *new_hashmap = malloc(sizeof(Hashmap));
	memcpy(new_hashmap, &hashmap_init, sizeof(Hashmap));

	new_hashmap->buckets = malloc(sizeof(HashmapEntry *) * capacity);
	memset(new_hashmap->buckets, 0, sizeof(HashmapEntry *) * capacity);

	new_hashmap->head = NULL;
	new_hashmap->tail = NULL;

	return new_hashmap;
}

//--------------------------------------------------------------------- MAIN ---
int main(void)
{
	uint32_t seed = 31;
	// Hashmap *hashmap = new_hashmap(18, seed, murmurhash3_x86_128);

	//----------------------------------------------------------- fib hash
	char *dummy_key       = "abb";
	const uint16_t *chunk = (const uint16_t *)dummy_key;
	const uint32_t simple = (const uint32_t)*dummy_key;

	printf("%u : %s\n", *chunk, (char *)chunk);
	printf("%u : %s\n", simple, (char *)&simple);

	for (int i = 7; i >= 0; i--) {
		putchar(((*dummy_key + 1) & (1 << i)) ? '1' : '0');
	}

	// size_t stringbytes = 0;

	// for (size_t i = 0; i <strlen(dummy_key); i++)
	// int i = 0;
	size_t hash     = seed;
	size_t hash_odd = seed;
	size_t hash_fimur = seed;
	while (*dummy_key) {
		putchar(*dummy_key);
		// stringbytes |= *(dummy_key++) << 8 * i++;
		hash += *(dummy_key++) * 11400714819323198485llu;
		hash |= 1;
	}
	// hash >>= 54;

	// printf("\nstringbytes : %lu\n", stringbytes);
	printf("\nhash : %lu\n", hash);

	Hash128 hash_murmur;
	size_t hash_murmur_reduced;

	size_t n = 8;

#define KEYPOOL_SIZE 9
	char random_keys[KEYPOOL_SIZE] = {'\0'};
	size_t test_count              = 256;

	// for (size_t i = 0; i < KEYPOOL_SIZE - 1; i++) {
	// 	random_keys[i] = (char)(i + 0x21);
	// }
	for (size_t k = 0; k < test_count; k++) {
		//---------------------------------------------------- setup
		for (size_t i = 0; i < KEYPOOL_SIZE - 1; i++) {
			random_keys[i] = (char)(rand() % 2 + 0x61);
		}

		dummy_key = random_keys;

		//-------------------------------------------------- murmur3
		hash_murmur = murmurhash3_x86_128(dummy_key, strlen(dummy_key), seed);
		hash_murmur_reduced = mod_hash128(hash_murmur, n);

		//----------------------------------- fibonhash + fiboddhash

		hash     = seed;
		hash_odd = seed;
		hash_fimur = seed;
		while (*dummy_key) {
		putchar(*dummy_key);
		// stringbytes |= *(dummy_key++) << 8 * i++;
		hash *= 11400714819323198485llu;
		hash += *(dummy_key);
		hash_odd *= 11400714819323198485llu * (*(dummy_key) | 1);
		// hash_odd |= 1;
		hash_fimur *= 11400714819323198485llu;
		hash_fimur *= *(dummy_key++) << 15;
		// hash_xor *= *(dummy_key) << 15;
		hash_fimur = (hash_fimur << 7) | (hash_fimur >> (32 - 7));
		}

		printf(" %lu %lu %lu %lu %lu %lu %lu%lu %lu\n",
		       hash,
		       hash >> (64 - n),
		       hash_odd,
		       hash_odd >> (64 - n),
		       hash_fimur,
		       hash_fimur >> (64 - n),
		       hash_murmur.hi,
		       hash_murmur.lo,
		       hash_murmur_reduced);
	}

	//-------------------------------------------------------------------- setup
	// #define KEYPOOL_SIZE 32
	// 	char random_keys[KEYPOOL_SIZE] = {'\0'};
	// 	size_t test_count              = 10;
	// 	char key[256];
	// 	char *dummy_value;

	// 	for (size_t k = 0; k < test_count; k++) {
	// 		for (size_t i = 0; i < KEYPOOL_SIZE - 1; i++) {
	// 			random_keys[i] = (char)(rand() % 26 + 0x61);
	// 			// putchar(random_keys[i]);
	// 		}
	// 		dummy_value = strdup(&random_keys[rand() % (KEYPOOL_SIZE - 1)]);
	// 		// printf("%s : %s\n",
	// 		//        &random_keys[rand() % (KEYPOOL_SIZE - 1)],
	// 		//        dummy_value);
	// 		put_hashmap(hashmap,
	// 		            &random_keys[rand() % (KEYPOOL_SIZE - 1)],
	// 		            dummy_value,
	// 		            free);
	// 		// free(dummy_value);
	// 	}

	// 	//----------------------------------------------------------- input loop
	// 	// dump_hashmap(hashmap);

	// 	for (;;) {
	// 		fputs("\x1b[102m > \x1b[0m", stdout);
	// 		scanf("%s", key);

	// 		if ((strcmp(key, "exit")) == 0) {
	// 			break;
	// 		}

	// 		if ((strcmp(key, "dump")) == 0) {
	// 			dump_hashmap(hashmap);
	// 		}
	// 		else {
	// 			dummy_value = strdup(key);
	// 			printf("%s : %s\n", key, dummy_value);
	// 			put_hashmap(hashmap, key, dummy_value, free);
	// 			// putchar('\n');
	// 			// printf("%32s : %s\n", hashmap->tail->key, (char
	// 			// *)hashmap->tail->value);
	// 			// putchar('\n');
	// 			print_hashmap(hashmap);
	// 		}
	// 	}

	// 	//-------------------------------------------------------------- cleanup
	// 	delete_hashmap(hashmap);
	return 0;
}
