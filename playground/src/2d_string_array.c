#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
	size_t row_count = 16;
	size_t col_count = 16;
	size_t two_d_size  = row_count * col_count;

	char * const two_d_string_array = malloc(sizeof(char) * two_d_size);
	memset(two_d_string_array, '\0', sizeof(char) * two_d_size);

	// // individual string NOT \0 terminated, whole array only
	// two_d_string_array = malloc(sizeof(char) * (row_count * col_count) + 1);

	// // individual string \0 terminated, better get row_count at 2^n - 1
	// two_d_string_array = malloc(sizeof(char) * (row_count + 1) * col_count);

	// individual string \0 terminated, restrict strings to row_count - 1 size !
	// two_d_string_array = malloc(sizeof(char) * row_count * col_count);

	char buffer[16];
	
	char *two_d_index = two_d_string_array;
	for (size_t i = 0; i < 16; i++, two_d_index += col_count) {
		snprintf(buffer, col_count, "[%lu]\thello", i);
		strncpy(two_d_index, buffer, col_count);
	}


	two_d_index = two_d_string_array;
	for (size_t i = 0; i < 16; i++, two_d_index += col_count) {
		printf("%s\n", two_d_index);
	}	
	

	free(two_d_string_array);

	return 0;
}
