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
#define ARRAY_LENGTH(array) (sizeof(array) / sizeof((array)[0]))

//----------------------------------------------------- FORWARD DECLARATIONS ---
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
	ValueDestructor destructor;
	struct HashmapEntry *bucket_next; /* in bucket */
	struct HashmapEntry *previous;    /* in insertion list */
	struct HashmapEntry *next;        /* in insertion list */
} HashmapEntry;

typedef struct Hashmap {
	const uint32_t seed;
	HashFunc function;
	unsigned int n;
	size_t capacity;
	size_t count;
	HashmapEntry **buckets;
	HashmapEntry *head; /* insertion list */
	HashmapEntry *tail; /* insertion list */
} Hashmap;

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
 *   - [ ] Figure out how bad it is to let caller allocate for value and
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

	new_entry->value       = alloced_value;
	new_entry->destructor  = destructor;
	new_entry->hash        = hash;
	new_entry->bucket_next = NULL;
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

	HashmapEntry *entry = hashmap->buckets[index];

	/* Empty bucket */
	if (entry == NULL) {
		hashmap->buckets[index] =
		    new_hashmap_entry(key, alloced_value, destructor, hash);
		// check_hashmap(hashmap);
		/* Append to  insertion list */
		hashmap->count++;
		if (hashmap->tail == NULL) {
			hashmap->head                     = hashmap->buckets[index];
			hashmap->buckets[index]->previous = NULL;
		}
		else {
			hashmap->tail->next               = hashmap->buckets[index];
			hashmap->buckets[index]->previous = hashmap->tail;
		}
		hashmap->buckets[index]->next = NULL;
		hashmap->tail                 = hashmap->buckets[index];
		return;
	}

	/* Collision */
	HashmapEntry *current = entry;
	// while (entry != NULL) {
	do {
		if (strcmp(entry->key, key) == 0) {
			entry->destructor(entry->value);
			entry->value = alloced_value;
			return;
		}
		current = entry;

	} while ((entry = entry->bucket_next) != NULL);

	current->bucket_next =
	    new_hashmap_entry(key, alloced_value, destructor, hash);
	hashmap->count++;
	hashmap->tail->next            = current->bucket_next;
	current->bucket_next->previous = hashmap->tail;
	current->bucket_next->next     = NULL;
	hashmap->tail                  = current->bucket_next;

	// check_hashmap(hashmap);
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
 * Free HashmapEntry of given Hashmap for given key
 *   -> nothing
 */
void delete_hashmap_entry(Hashmap *hashmap, const char *key)
{
	const Hash128 hash = hashmap->function(key, strlen(key), hashmap->seed);
	const size_t index = mod_hash128(hash, hashmap->n);
	destroy_entry(hashmap->buckets[index]);
}

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
			next = entry->bucket_next;
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
 *   - [ ] Consider a insertion ordered double-linked list of HashmapEntry
 */
void dump_hashmap(Hashmap *hashmap)
{
	// for (size_t i = 0; i < hashmap->capacity; i++) {
	// 	HashmapEntry *entry = hashmap->buckets[i];

	// 	if (entry == NULL) {
	// 		continue;
	// 	}

	// 	printf("hashmap->bucket[%lu]\n", i);

	// 	for (;;) {
	// 		printf("\t %016lx %016lx : %s\n",
	// 		       entry->hash.hi,
	// 		       entry->hash.lo,
	// 		       entry->key);
	// 		if (entry->bucket_next == NULL) {
	// 			break;
	// 		}
	// 		entry = entry->bucket_next;
	// 	}
	// 	// putchar('\n');
	// }
	printf("hashmap->seed     : %u\n", hashmap->seed);
	printf("hashmap->function : %p\n", (unsigned char *)&(hashmap->function));
	printf("hashmap->n        : %u\n", hashmap->n);
	printf("hashmap->capacity : %lu\n", hashmap->capacity);
	printf("hashmap->count    : %lu\n", hashmap->count);
	printf("hashmap->buckets  : %p\n", (void *)hashmap->buckets);
	printf("hashmap->head     : %p\n", (void *)hashmap->head);
	printf("hashmap->tail     : %p\n", (void *)hashmap->tail);
}

//----------------------------------------------------------------- Function ---
/**
 * Print given Hashmap content
 *   -> nothing
 */
void print_hashmap(Hashmap *hashmap)
{
	HashmapEntry *entry = hashmap->head;

	while (entry != NULL) {
		printf("%32s : %s\n", entry->key, (char *)entry->value);
		entry = entry->next;
	}
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
	    seed, function, n, capacity, 0, NULL, NULL, NULL};

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
	uint32_t seed    = 31;
	Hashmap *hashmap = new_hashmap(17, seed, murmurhash3_x86_128);

//------------------------------------------------------------ setup
#define KEYPOOL_SIZE 32
	char random_keys[KEYPOOL_SIZE] = {'\0'};
	size_t test_count              = 100000;
	char key[256];
	char *dummy_value;

	// putchar('\n');

	for (size_t k = 0; k < test_count; k++) {
			for (size_t i = 0; i < KEYPOOL_SIZE - 1; i++) {
				random_keys[i] = (char)(rand() % 26 + 0x61);
				// putchar(random_keys[i]);
			}
			dummy_value = strdup(&random_keys[rand() % (KEYPOOL_SIZE - 1)]);
			// printf("%s : %s\n",
			//        &random_keys[rand() % (KEYPOOL_SIZE - 1)],
			//        dummy_value);
			put_hashmap(hashmap,
			            &random_keys[rand() % (KEYPOOL_SIZE - 1)],
			            dummy_value,
			            free);
			// free(dummy_value);
	}
	//------------------------------------------------------ interactive

	// dump_hashmap(hashmap);

	for (;;) {
		scanf("%s", key);
		if ((strcmp(key, "exit")) == 0) {
			break;
		}

		dummy_value = strdup(key);
		printf("%s : %s\n", key, dummy_value);
		put_hashmap(hashmap, key, dummy_value, free);
		dump_hashmap(hashmap);
		putchar('\n');
		printf("%32s : %s\n", hashmap->tail->key, (char *)hashmap->tail->value);
		putchar('\n');
		// print_hashmap(hashmap);
	}

	//-------------------------------------------------------------- cleanup
	delete_hashmap(hashmap);
	return 0;
}
