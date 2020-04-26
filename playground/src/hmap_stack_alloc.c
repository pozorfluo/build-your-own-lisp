#include <stddef.h> /* size_t */
#include <stdio.h>
#include <string.h> /* memcpy */

//------------------------------------------------------------------- MACROS ---
// #define FIRST(a, ...) a
// #define SECOND(a, b, ...) b

// #define EMPTY()

// #define EVAL1024(...) EVAL512(__VA_ARGS__), EVAL512(__VA_ARGS__)
// #define EVAL512(...) EVAL256(__VA_ARGS__), EVAL256(__VA_ARGS__)
// #define EVAL256(...) EVAL128(__VA_ARGS__), EVAL128(__VA_ARGS__)
// #define EVAL128(...) EVAL64(__VA_ARGS__), EVAL64(__VA_ARGS__)
// #define EVAL64(...) EVAL32(__VA_ARGS__), EVAL32(__VA_ARGS__)
// #define EVAL32(...) EVAL16(__VA_ARGS__), EVAL16(__VA_ARGS__)
// #define EVAL16(...) EVAL8(__VA_ARGS__), EVAL8(__VA_ARGS__)
// #define EVAL8(...) EVAL4(__VA_ARGS__), EVAL4(__VA_ARGS__)
// #define EVAL4(...) EVAL2(__VA_ARGS__), EVAL2(__VA_ARGS__)
// #define EVAL2(...) EVAL1(__VA_ARGS__), EVAL1(__VA_ARGS__)
// #define EVAL1(...) __VA_ARGS__

// #define LIST 1

#define ARRAY_LENGTH(_array) (sizeof(_array) / sizeof((_array)[0]))
//------------------------------------------------------------- DECLARATIONS ---
typedef signed char meta_byte;

enum meta_ctrl {
	META_EMPTY    = -128, /* 0b10000000 */
	META_OCCUPIED = 0,    /* 0b0xxxxxxx */
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

struct hmap {
	struct hmap_buckets buckets;
	struct hmap_entry *store;
	size_t top;
	size_t hash_shift; /* shift amount necessary for desired hash depth */
	size_t capacity;   /* now is actual capacity */
	size_t count;      /* occupied entries count */
};

//--------------------------------------------------------------------- MAIN ---
int main(void)
{
	/* this will require just shy of 7MB on the stack */
#define HMAP_SIZE (1 << 18)
	struct hmap_buckets buckets = {(meta_byte[HMAP_SIZE]){1},
	                               (meta_byte[HMAP_SIZE]){2},
	                               (size_t[HMAP_SIZE]){3}};
	struct hmap_entry entry = {4, 5};
	struct hmap hashmap     = {
	    buckets, (struct hmap_entry[HMAP_SIZE]){entry}, 0, 7, HMAP_SIZE, 0};

	for (size_t i = 0; i < HMAP_SIZE; i++) {
		printf("[%16lu] %02hhu : %02hhu : %lu | %lu : %lu\n",
		       i,
		       hashmap.buckets.metas[i],
		       hashmap.buckets.distances[i],
		       hashmap.buckets.entries[i],
		       hashmap.store[i].key,
		       hashmap.store[i].value);
	}
	/* [HMAP_SIZE] -> 1 slot after the last element of the array */
	printf("size of hashmap    = %lu\n", sizeof(hashmap));
	printf("size of metas      = %lu\n",
	       (char *)(&hashmap.buckets.metas[HMAP_SIZE]) -
	           (char *)(&hashmap.buckets.metas[0]));
	printf("size of distances  = %lu\n",
	       (char *)(&hashmap.buckets.distances[HMAP_SIZE]) -
	           (char *)(&hashmap.buckets.distances[0]));
	printf("size of entries    = %lu\n",
	       (char *)(&hashmap.buckets.entries[HMAP_SIZE]) -
	           (char *)(&hashmap.buckets.entries[0]));

	printf("size of store      = %lu\n",
	       (char *)&(hashmap.store[HMAP_SIZE]) - (char *)&(hashmap.store[0]));
	printf("size of top        = %lu\n", sizeof(hashmap.top));
	printf("size of hash_shift = %lu\n", sizeof(hashmap.hash_shift));
	printf("size of capacity   = %lu\n", sizeof(hashmap.capacity));
	printf("size of count      = %lu\n", sizeof(hashmap.count));

	// printf("%p\n", (void*)&(hashmap.buckets));
	// printf("%p\n", (void*)&(hashmap.store));
	// printf("%p\n", (void*)&(hashmap.top));
	// printf("%p\n", (void*)&(hashmap.hash_shift));
	// printf("%p\n", (void*)&(hashmap.capacity));
	// printf("%p\n", (void*)&(hashmap.count));
	// printf("%p\n", (void*)&(hashmap.buckets));
	// printf("%p\n", (void*)&(hashmap.buckets.metas[0]));
	// printf("%p\n", (void*)&(hashmap.buckets.distances[0]));
	// printf("%p\n", (void*)&(hashmap.buckets.entries[0]));
	// printf("%p\n", (void*)&(hashmap.store[0]));
	// printf("%p\n", (void*)&(hashmap.store[0].key));

	// int tab[] = {EVAL1024(1, 2, 3, 4)};
	// int tab[] = {[0 ... HMAP_SIZE] = 7};

	// printf("size of tab[] = %lu\n", ARRAY_LENGTH(tab));

	// for (size_t i = 0; i < ARRAY_LENGTH(tab); i++) {
	// 	printf("\ttab[%lu] = %d\n", i, tab[i]);
	// }

	// struct entry stack_entry        = {1, 5};
	// typeof(stack_entry) another_one = {15, 10};

	// printf("%lu : %lu | %lu : %lu/n",
	//        stack_entry.key,
	//        stack_entry.value,
	//        another_one.key,
	//        another_one.value);
	// struct entry* stack_entry = $(typeof(stack_entry), 10, 15);
	return 0;
}