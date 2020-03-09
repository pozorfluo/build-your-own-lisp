// #include <immintrin.h>
#include <x86intrin.h> /* __rdtsc() */
// #include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum meta_ctrl { CTRL_EMPTY = -128, CTRL_DELETED = -1, CTRL_TOMBSTONE = -2 };

typedef signed char ctrl_byte;

static inline size_t hash_index(const size_t hash) { return hash >> 8; }

static inline ctrl_byte hash_meta(const size_t hash) { return hash & 0xFF; }

void print_m128i(const __m128i value)
{
	printf("%016llx%016llx\n", value[1], value[0]);
}

void print_m128i_hexu8(const __m128i value)
{
	// _Alignas(uint16_t) uint16_t vec[8];
	uint8_t vec[16];
	_mm_store_si128((__m128i *)vec, value);
	printf(
	    "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x "
	    "%02x %02x\n",
	    vec[0],
	    vec[1],
	    vec[2],
	    vec[3],
	    vec[4],
	    vec[5],
	    vec[6],
	    vec[7],
	    vec[8],
	    vec[9],
	    vec[10],
	    vec[11],
	    vec[12],
	    vec[13],
	    vec[14],
	    vec[15]);
}

//----------------------------------------------------------------- Function ---
/**
 * Pretty print bits for given n bytes at given data pointer
 *   Handle little endian only
 *   -> nothing
 */
void print_bits(const size_t n, void const *const data)
{
	unsigned char *bit = (unsigned char *)data;
	unsigned char byte;

	for (int i = n - 1; i >= 0; i--) {
		for (int j = 7; j >= 0; j--) {
			byte = (bit[i] >> j) & 1;
			printf("%u", byte);
		}
	}

	putchar('\n');
}

// void print_bits(const int number){
// 	if (number) {
// 		print_bits(number >> 1);
// 		putchar((number & 1) ? '1' : '0');
// 	}
// }

int main(void)
{
	//---------------------------------------------------------- split mock hash
	srand(__rdtsc());

	size_t hash = rand() % 0xffffffffffff;

	printf("%lx\n%lx : %02x\n\n", hash, hash_index(hash), hash_meta(hash));

	unsigned char entry_meta = hash_meta(hash);
	// size_t entry_index = hash_index(hash);

	//--------------------------------------------------------------- mock group
	puts("target group :");
	__m128i group = {hash, hash};
	print_m128i_hexu8(group);

	//------------------------------------------------------------- setup filter
	__m128i filter = _mm_set1_epi8(entry_meta);
	// print_m128i(filter);
	puts("filter :");
	print_m128i_hexu8(filter);

	//------------------------------------------------------------- filter group
	__m128i match = _mm_cmpeq_epi8(filter, group);
	puts("match :");
	print_m128i_hexu8(match);
	// print_bits(sizeof(match), &match);

	//-------------------------------------------------- turn matches to bitmask
	puts("match_mask :");
	int match_mask = _mm_movemask_epi8(match);
	printf("%032x\n", match_mask);
	print_bits(sizeof(match_mask), &match_mask);
	// print_bits(match_mask);

	//--------------------------------------------------------- mock many groups
	size_t mock_size           = 1024;
	unsigned char *const metas = malloc(sizeof(char) * mock_size);
	unsigned char *meta_index  = metas;

	for (size_t i = 0; i < mock_size; i++, meta_index++) {
		*meta_index = rand() % 0xff;
	}
	print_bits(mock_size, metas);
	putchar('\n');
	//-------------------------------------- filter in the middle of many groups
	filter = _mm_set1_epi8(entry_meta);
	puts("filter :");
	print_m128i_hexu8(filter);
	putchar('\n');

	meta_index = metas;

	for (size_t i = 0; i < mock_size; i += 16) {
		/**
		 * todo
		 *   - [ ] Study the difference : https://godbolt.org/z/zDTfwq
		 */
		// group = *(__m128i *)(meta_index + i);
		group = _mm_loadu_si128((__m128i *)(meta_index + i));
		// puts("target group :");
		print_m128i_hexu8(group);
		match = _mm_cmpeq_epi8(filter, group);
		// puts("match :");
		// print_m128i_hexu8(match);

		/**
		 * todo
		 * - [ ] Look for portable ways to pop count
		 *   + [ ] See :
		 * https://github.com/nemequ/portable-snippets/tree/master/builtin
		 */
		match_mask = _mm_movemask_epi8(match);

		//------------------------------------ Use bitmask to access matches
		while (match_mask != 0) {
			// int least_significant_set_bit_only = match_mask & -match_mask;
			size_t position = __builtin_ctzl(match_mask);
			printf("\t%x -> %d match @ [%lu]\n",
			       entry_meta,
			       _mm_popcnt_u32((unsigned int)match_mask),
			       position);
			//    _popcnt32((unsigned int)_mm_movemask_epi8(match)));
			//    __builtin_popcount(_mm_movemask_epi8(match)));
			match_mask ^= match_mask & -match_mask;
		}
		// putchar('\n');
	}

	// #define HAS_LESS(_x, _n) (((_x) - ~0ul / 255 * (_n)) & ~(_x) & ~0ul / 255
	// * 128)

	// // Tell GCC about most likely result
	// #ifdef __GNUC__
	// 	if (__builtin_expect(strcmp(entry.key, bucket->key) == 0, 1))
	// #else
	// 	if (strcmp(entry.key, bucket->key) == 0))
	// #endif
	// 	{
	// 	}
	free(metas);
	return 0;
}

