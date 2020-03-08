#include <stddef.h>

typedef signed char ctrl_byte;

enum ctrl_bit { CTRL_EMPTY = -128, CTRL_DELETED = -1};

// typedef size_t (*HashFunc)(const unsigned char *, const size_t *);

struct hmap_allocator {
	// void *(*malloc)(const void *);
	void *(*malloc)(size_t size);
	void (*free)(void *);
	// typedef void (*ValueDestructor)(void *, const char *, const char *);
};

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
	struct hmap_allocator allocator;
	unsigned int n;
	int probe_limit;
	size_t capacity;
	size_t count;
	struct hmap_stats stats;
};


int main(void)
{
    struct hmap hashy;
    return 0;
}