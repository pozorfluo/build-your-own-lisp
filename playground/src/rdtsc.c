#include <stdio.h>

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

int main()
{
    size_t seed = __rdtsc();
    printf("%lx", seed);

	return 0;
}

