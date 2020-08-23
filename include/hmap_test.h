//------------------------------------------------------------------ SUMMARY ---
/**
 * Test, debug tools for hmap.
 */

#ifndef HMAP_TEST_H
#define HMAP_TEST_H
#include <stddef.h> /* size_t */

void dump_hashmap(const struct hmap *const hm, size_t offset, size_t limit);
void dump_hashmap_horizontal(const struct hmap *const hm,
                             size_t offset,
                             size_t limit);

#endif /* HMAP_TEST_H */