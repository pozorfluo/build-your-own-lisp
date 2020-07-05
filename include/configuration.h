//------------------------------------------------------------------ SUMMARY ---
/**
 * Common configuration elements for various hmap implementations
 *
 */

#ifndef configuration_h
#define configuration_h

//------------------------------------------------------------ CONFIGURATION ---
#define WELCOME_MESSAGE                                                        \
	FG_BRIGHT_BLUE REVERSE                                                     \
	    " hmap version 0.26.2 " RESET FG_BRIGHT_BLUE                           \
	    " type exit to quit\n" RESET FG_BRIGHT_RED REVERSE                     \
	    "   todo \n" RESET FG_BRIGHT_RED                                       \
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
	    "  - [ ] Consider a compact alternative where small kvps are in a "    \
	    "separate array from metadata and are moved around. It saves 8 bytes " \
	    "from the pointer and fan indirection but loses store pointer "        \
	    "stability"                                                            \
	    "  - [ ] Consider that if you store the full hash instead of "         \
	    "pointer/index to the store you lose store stability, ease of "        \
	    "iteration, and will have to move kvp around on resize"                \
	    "  - [ ] Consider 1 full bytes of secondary hash in meta, 7 bits of "  \
	    "distance + 1 bit of ctrl for empty slots\n"                           \
	    "  - [ ] Consider for wraparound that some version of abseil hashmap " \
	    "replicates a probe sized chunk of data from the beginning at the "    \
	    "end\n"

#define SIMD_PROBE
#define BENCHMARK
#define TEST_COUNT 1000000
#include "benchmark.h"

#endif /* configuration_h */