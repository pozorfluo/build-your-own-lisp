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
	    "\t todo \n" RESET FG_BRIGHT_RED                                       \
	    "\t- [ ] Wrap up a version satisfying build-your-own-lisp use case\n"  \
	    "\t\t+ [ ] Implement <string => int>\n"                                \
	    "\t\t\t* [ ] Research ways to accomodate strings in the store\n"       \
	    "\t\t\t* [ ] Test\n"                                                   \
	    "\t\t\t* [ ] Bench different hash function\n"                          \
	    "\t\t+ [ ] Implement <string => pointer>\n"                            \
	    "\t\t\t* [ ] Test\n"                                                   \
	    "\t\t\t* [ ] Bench different hash function\n"                          \
	    "\t\t+ [ ] Implement <string => function pointer>\n"                   \
	    "\t\t\t* [ ] Test\n"                                                   \
	    "\t\t\t* [ ] Bench different hash function\n"                          \
	    "\t\t+ [ ] Investigate errors with very long collision chains on "     \
	    "full table\n"                                                         \
	    "\t\t+ [ ] Research hashtable resizing strategy\n"                     \
	    "\t\t\t* [ ] Implement one, move on\n"                                 \
	    "\t- [ ] Move on to next build your own lisp step\n" RESET             \
	        FG_BRIGHT_YELLOW                                                   \
	    "\t- [ ] Bench against array\n"                                        \
	    "\t\t+ [ ] Find break even point for hmap vs array\n"

#define SIMD_PROBE
#define BENCHMARK
#define TEST_COUNT 1000000
#include "benchmark.h"

#endif /* configuration_h */