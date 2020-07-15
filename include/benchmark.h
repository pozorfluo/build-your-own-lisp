//------------------------------------------------------------------ SUMMARY ---
/**
 * Naive benchmark setup
 *
 * usage
 *   SETUP_BENCH(unique_name);
 *   START_BENCH(unique_name);
 *   STOP_BENCH(unique_name);
 */

//---------------------------------------------------------------- BENCHMARK ---
#ifndef BENCHMARK_H
#define BENCHMARK_H

#ifdef BENCHMARK
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#include <x86intrin.h>
#define SETUP_BENCH(_prefix)                                                   \
	float _prefix##_start, _prefix##_stop, _prefix##_diff, _prefix##_result;

#define START_BENCH(_prefix)                                                   \
	do {                                                                       \
		_prefix##_start = (float)clock() / CLOCKS_PER_SEC;                     \
	} while (0)

#define STOP_BENCH(_prefix)                                                    \
	do {                                                                       \
		_prefix##_stop   = (float)clock() / CLOCKS_PER_SEC;                    \
		_prefix##_diff   = _prefix##_stop - _prefix##_start;                   \
		_prefix##_result = _prefix##_diff;                                     \
		printf("|><| %f s\n", _prefix##_result);                               \
	} while (0)

#else
#define SETUP_BENCH(_prefix)
#define START_BENCH(_prefix)
#define STOP_BENCH(_prefix)
#endif /* BENCHMARK */
#endif /* BENCHMARK_H */