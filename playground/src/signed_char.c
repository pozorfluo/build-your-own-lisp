#include <stdio.h>

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

int main(void){
    char minus128 = -128;
    char minus1 = -1;
    char minus2 = 'q';
	print_bits(sizeof(minus128), &minus128);
	print_bits(sizeof(minus1), &minus1);
	print_bits(sizeof(minus2), &minus2);
    printf("%c\n", (unsigned char)minus128);
    printf("%d\n", (unsigned char)minus128);
    printf("%d\n", minus128);
    return 0;
}