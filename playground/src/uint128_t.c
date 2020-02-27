#include <stdio.h>
#include <stdint.h>   // for uint64_t

#define uint128_t __uint128_t
//#define uint128_t unsigned __int128

uint128_t mul64(uint64_t a, uint64_t b) {
    return (uint128_t)a * b;
}

int main()
{
	uint128_t hu128 = mul64( 321545654465ull, 4545578663ull);
	// printf("\n %lu", hu128);
	return 0;
}
