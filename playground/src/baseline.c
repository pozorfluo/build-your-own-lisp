#include <stddef.h> /* size_t */
#include <stdint.h> /* uint32_t, uint64_t */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ansi_esc.h"

struct hmap_entry {
	size_t key;
	size_t value;
};

//--------------------------------------------------------------------- MAIN ---
int main(void)
{

	size_t n = 8;

	fputs(FG_BRIGHT_BLUE REVERSE
	      "Table size is 2^n. Enter n ( default n=8 ) " FG_BRIGHT_BLUE
	      " ? " RESET,
	      stdout);
	scanf("%lu", &n);

	size_t capacity = (n < 1) ? (1u << 1) : (1u << n);
	struct hmap_entry *const hashmap =
	    malloc(sizeof(struct hmap_entry) * capacity);

	//-------------------------------------------------------------------- setup

	size_t test_count = (1 << n) * 0.98; // 1 << (n - 1);
	char key[256];

	printf(FG_BRIGHT_YELLOW REVERSE "Filling hashmap with %lu entries\n" RESET,
	       test_count);

	for (size_t k = 0; k < test_count; k++) {
		hashmap[k].key   = k;
		hashmap[k].value = k;
	}
	printf(FG_BRIGHT_YELLOW REVERSE "Done !\n" RESET);
	//----------------------------------------------------------- input loop
	for (;;) {
		fputs("\x1b[102m > \x1b[0m", stdout);
		// scanf("%s", key);
		fgets(key, 255, stdin);
		size_t length = strlen(key);

		/* trim newline */
		if ((length > 0) && (key[--length] == '\n')) {
			key[length] = '\0';
		}

		if ((strcmp(key, "exit")) == 0) {
			break;
		}

		if ((strcmp(key, "rm")) == 0) {
			for (size_t k = 0; k < test_count; k++) {
				hashmap[k].key   = 0;
				hashmap[k].value = 0;
			}
			continue;
		}

		if ((strcmp(key, "fill")) == 0) {
			for (size_t k = 0; k < test_count; k++) {
				hashmap[k].key   = k;
				hashmap[k].value = k;
			}
			continue;
		}

		if ((strcmp(key, "dump")) == 0) {
			for (size_t k = 0; k < test_count; k++) {
				printf("%lu | %lu\n", hashmap[k].key, hashmap[k].value);
			}
			continue;
		}

		if ((strcmp(key, "sum")) == 0) {
			size_t sum_key = 0;
			size_t sum_value = 0;
			for (size_t k = 0; k < test_count; k++) {
				sum_key +=  hashmap[k].key;
				sum_value +=  hashmap[k].value;
			}
			printf("sum : %lu | %lu\n", sum_key, sum_value);
			continue;
		}

	}

	//-------------------------------------------------------------- cleanup
	free(hashmap);
	return 0;
}