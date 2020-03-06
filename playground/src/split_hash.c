#include <stdio.h>
#include <stddef.h>

typedef enum ctrl_bit {
    EMPTY = -128,
    DELETED = -2,
    TOMBSTONE = -1,
} ctrl_bit;


static inline size_t hash_index(const size_t hash) {
    return hash >> 8;
}

static inline ctrl_bit hash_meta(const size_t hash) {
    return hash & 0xFF;
}


int main (void) {
    size_t hash = 0xAABBCCDDEE07;

    printf("%lx\n%lx : %02x\n", hash, hash_index(hash), hash_meta(hash));
}

