#include <limits.h> /* UINT_MAX */
#include <stddef.h> /* size_t */
#include <stdint.h> /* uint32_t, uint64_t */
#include <stdio.h>

#include "hfunc.h"

#define HMAP_INLINE_KEY_SIZE 16
// #define HFIBO 11400714819323198485llu
#define HSHIFT (64 - 8)
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

// //----------------------------------------------------------------- Function
// ---
// static inline size_t reduce_fibo(const size_t hash, c	onst size_t shift)
// {
// 	const size_t xor_hash = hash ^ (hash >> shift);
// 	return (HFIBO * xor_hash) >> shift;
// }
//--------------------------------------------------------------------- MAIN ---
int main(void)
{
	// char buffer[HMAP_INLINE_KEY_SIZE] = "0123456789abcdef";
	// char buffer[HMAP_INLINE_KEY_SIZE] = "abcd\0\0\0\0\0\0\0\0\0\0\0\0";
	char buffer[HMAP_INLINE_KEY_SIZE] = "qyvoqerqvnzvarjd";

	printf("__WORDSIZE %d\n", __WORDSIZE);
	printf("SIZE_MAX  %lu\n", SIZE_MAX);
	printf("UINT_MAX  %u\n", UINT_MAX);
	printf("UINT64_MAX  %lu\n", UINT64_MAX);

	printf(
	    "buffer\n"
	    "\t as str   : %.*s\n",
	    HMAP_INLINE_KEY_SIZE,
	    buffer);

	print_bits(HMAP_INLINE_KEY_SIZE / 2, buffer);
	print_bits(HMAP_INLINE_KEY_SIZE / 2, buffer + HMAP_INLINE_KEY_SIZE / 2);
	*(size_t *)buffer = SIZE_MAX;
	print_bits(HMAP_INLINE_KEY_SIZE / 2, buffer);

	size_t k1 = *(size_t *)buffer;
	size_t k2 = *(size_t *)(buffer + HMAP_INLINE_KEY_SIZE / 2);
	size_t kr = k1 ^ k2;

	printf(
	    "kr   : %lu\n"
	    "hash : %lu\n"
	    "hash : %lu\n",
	    kr,
	    reduce_fibo(kr, HSHIFT),
	    hash_multiplicative(buffer, HMAP_INLINE_KEY_SIZE));

	buffer[15] = 'g';
	k1         = *(size_t *)buffer;
	k2         = *(size_t *)(buffer + HMAP_INLINE_KEY_SIZE / 2);
	kr         = k1 ^ k2;

	printf(
	    "kr   : %lu\n"
	    "hash : %lu\n"
	    "hash : %lu\n",
	    kr,
	    reduce_fibo(kr, HSHIFT),
	    hash_multiplicative(buffer, HMAP_INLINE_KEY_SIZE));

	return 0;
}