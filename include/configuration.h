//------------------------------------------------------------------ SUMMARY ---
/**
 * Common configuration elements for various hmap implementations
 *
 */

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

//------------------------------------------------------------ CONFIGURATION ---
// #define SIMD_PROBE
#define HMAP_INLINE_KEY_SIZE 16
#define HFUNC hash_fixed128
#define HREDUCE reduce_fibo
#define HCMP strncmp
#define HCOPY strcpy
#define BENCHMARK
// #define TEST_REGISTER
#define TEST_COUNT 1000000
#include "benchmark.h"

#define WELCOME_MESSAGE                                                        \
	FG_BRIGHT_BLUE REVERSE                                                     \
	    " hmap version 0.26.3 " RESET FG_BRIGHT_BLUE                           \
	    " type exit to quit\n" RESET FG_BRIGHT_RED REVERSE                     \
	    "   todo \n" RESET FG_BRIGHT_RED                                       \
	    "  - [ ] Reconsider tab_hash\n"                                        \
	    "  - [x] Consider reading fixed size keys as n size_t\n"               \
	    "    + [x] Use it for HFUNC\n"                                         \  
	    "    + [ ] Use it for HCMP\n"                                          \
	    "  - [ ] Decide on return value for key not found on hmap_get\n"       \
	    "  - [ ] Handle any key size lower or equal to HMAP_INLINE_KEY_SIZE\n" \
	    "    + [ ] Replace memcpy on put\n"                                    \
	    "    + [ ] Replace memcmp on find\n"                                   \
	    "      * [ ] Research efficient alt shortcircuit to compare\n"         \
	    "    + [ ] Think about a useful thing to do or not do on delete\n"     \
	    "  - [ ] Wrap up a version satisfying build-your-own-lisp use case\n"  \
	    "    + [ ] Set up a interface with tentative implementation asap\n"    \
	    "    + [ ] Implement <string => int>\n"                                \
	    "      * [ ] Research ways to accomodate strings in the store\n"       \
	    "      * [ ] Consider making fixed size of inlined key a parameter "   \
	    "of hmap_new \n"                                                       \
	    "      * [ ] Test\n"                                                   \
	    "      * [ ] Bench different hash function\n"                          \
	    "    + [ ] Implement <string => pointer>\n"                            \
	    "      * [ ] Test\n"                                                   \
	    "      * [ ] Bench different hash function\n"                          \
	    "    + [ ] Implement <string => function pointer>\n"                   \
	    "      * [ ] Test\n"                                                   \
	    "      * [ ] Bench different hash function\n"                          \
	    "    + [ ] Investigate errors with very long collision chains on "     \
	    "full table\n"                                                         \
	    "    + [ ] Research hashtable resizing strategy\n"                     \
	    "      * [ ] Implement one, move on\n"                                 \
	    "      * [ ] Consider separately allocing the store and buckets to  "  \
	    "allow resizing in place\n"                                            \
	    "  - [ ] Move on to next build your own lisp step\n" RESET             \
	        FG_BRIGHT_YELLOW                                                   \
	    "  - [ ] Bench against array\n"                                        \
	    "    + [ ] Find break even point for hmap vs array\n" RESET            \
	        FG_BRIGHT_GREEN                                                    \
	    "  - [ ] Consider unions for k and v of types up to size of pointer "  \
	    "+ size_t for data length\n"                                           \
	    "  - [ ] Consider parameterizing size of k and v\n"                    \
	    "  - [ ] Consider a compact alternative where small kvps are in a "    \
	    "separate array from metadata and are moved around. It saves 8 bytes " \
	    "from the pointer and fan indirection but loses store pointer "        \
	    "stability\n"                                                          \
	    "  - [ ] Consider that if you store the full hash instead of "         \
	    "pointer/index to the store you lose store stability, ease of "        \
	    "iteration, and will have to move kvp around on resize\n"              \
	    "  - [ ] Consider 1 full bytes of secondary hash in meta, 7 bits of "  \
	    "distance + 1 bit of ctrl for empty slots\n"                           \
	    "  - [ ] Consider for wraparound that some version of abseil hashmap " \
	    "replicates a probe sized chunk of data from the beginning at the "    \
	    "end\n"

#endif /* CONFIGURATION_H */