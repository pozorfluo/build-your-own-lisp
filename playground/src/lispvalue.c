#include <stdio.h>
#include <string.h>

//-------------------------------------------------------------- ANSI MACROS ---

// #pragma pack(push)  /* push current alignment to stack */
// #pragma pack(1)     /* set alignment to 1 byte boundary */

// #define PAD_SIZE 128

#define PRINT_PRETTY_SIZE(_msg, _expression)                                   \
	printf(_msg "\t: %lu \tbytes %s\n",                                        \
	       sizeof(_expression),                                                \
	       pretty_byte - sizeof(_expression));

#define PRINT_PRETTY_AT(_msg, _var)                                            \
	printf(_msg "\t: %lu \tbytes\t@ %p %s\n",                                  \
	       sizeof(_var),                                                       \
	       (void *)&_var,                                                      \
	       pretty_byte - sizeof(_var));
//----------------------------------------------------------------- TYPEDEFS ---
typedef struct LispValue {
	int type;
	double number;
	char *error;
	char *symbol;
	/* sexpr */
	int count;
	struct LispValue **cells;
} LispValue;

typedef struct LispValueReordered {
	int type;
	int count;
	double number;
	char *error;
	char *symbol;
	struct LispValue **cells;
} LispValueReordered;
// } __attribute__((packed)) LispValue;

// #pragma pack(pop) /* resume normal padding */

//----------------------------------------------------------------- Function ---
// char *prettify_bytes(size_t count)
// {
// 	char *pretty_bytes =
// 	    "################################################################";

// 	pretty_bytes += strlen(pretty_bytes) - count;
// 	// printf("%s\n", pretty_bytes);

// 	// return pretty_bytes; /* undefined behaviour, correct ? */
// 	return strdup(pretty_bytes);
// 	// /* another way */
// 	// char pretty_bytes_array[] =
// 	//     "################################################################";
// 	// /* place a nul char to terminate the string after count length */
// 	// /* remember array 0 index */
// 	// pretty_bytes_array[count] = '\0';
// 	// printf("%s\n", pretty_bytes_array);

// 	// /* playing with printf format, but not getting a string back */
// 	// printf("%s %lu\n", pretty_bytes, count);
// 	// printf(FG_BLACK BG_YELLOW " % *d" RESET "\n", (int)size, (int)size);
// 	// size *= 2;
// 	// printf(FG_BLACK BG_BLUE " % *d" RESET "\n", (int)size, (int)size);

// 	// memset(dest, '\0', sizeof(dest));

// 	// char sz[] = "00000000000000000000";
// 	// char *a   = "1234";
// 	// strcpy(sz + strlen(sz) - strlen(a), a);
// 	// printf("%s\n%s\n", sz, a);
// }

//--------------------------------------------------------------------- MAIN ---
int main()
{
	// /* char array on the stack can be modified */
	// char padding[PAD_SIZE + 1];
	// memset(padding, '#', sizeof(char) * PAD_SIZE);
	// padding[PAD_SIZE] = '\0';

	/* string literal in read-only memory section */
	char *padding =
	    "################################################################";
	char *pretty_byte = padding + strlen(padding);

	// printf("padding \t\t@ %p\n", padding);
	// printf("pretty_byte \t\t@ %p\n", pretty_byte);
	// printf("%lu\n", strlen(padding));
	// printf("%s\n", padding);
	// printf("%s\n", pretty_byte);

	puts("info about struct LispValue");
	// puts(padding);

	size_t total_unpadded_size = sizeof(int) + sizeof(double) + sizeof(char *) +
	                             sizeof(char *) + sizeof(int) +
	                             sizeof(LispValue **);

	PRINT_PRETTY_SIZE("struct LispValue     \t", LispValue);
	printf("\ttotal_unpadded_size \t: %lu \tbytes %s\n",
	       total_unpadded_size,
	       pretty_byte - total_unpadded_size);
	PRINT_PRETTY_SIZE("\t(int) type           ", int);
	PRINT_PRETTY_SIZE("\t(double) number      ", double);
	PRINT_PRETTY_SIZE("\t(char *) error       ", char *);
	PRINT_PRETTY_SIZE("\t(char *) symbol      ", char *);
	PRINT_PRETTY_SIZE("\t(int) count          ", int);
	PRINT_PRETTY_SIZE("\t(LispValue **) cells ", LispValue **);

	puts("\nLispValue lispvalue");
	LispValue lispvalue_array[2];

	PRINT_PRETTY_AT("LispValue            \t", lispvalue_array[0]);
	PRINT_PRETTY_AT("\t(int) type           ", (lispvalue_array[0].type));
	PRINT_PRETTY_AT("\t(double) number      ", (lispvalue_array[0].number));
	PRINT_PRETTY_AT("\t(char *) error       ", (lispvalue_array[0].error));
	PRINT_PRETTY_AT("\t(char *) symbol      ", (lispvalue_array[0].symbol));
	PRINT_PRETTY_AT("\t(int) count          ", (lispvalue_array[0].count));
	PRINT_PRETTY_AT("\t(LispValue **) cells ", (lispvalue_array[0].cells));

	puts("\nLispValue lispvalue");
	unsigned long padding_type;

	padding_type = (unsigned long)(&lispvalue_array[0].number) -
	               (unsigned long)(&lispvalue_array[0].type);
	printf("\n(int) type padded to \t\t: %lx bytes\n", padding_type);

	padding_type = (unsigned long)(&lispvalue_array[0].error) -
	               (unsigned long)(&lispvalue_array[0].number);
	printf("(double) number padded to \t: %lx bytes\n", padding_type);

	padding_type = (unsigned long)(&lispvalue_array[0].symbol) -
	               (unsigned long)(&lispvalue_array[0].error);
	printf("(char *) error padded to \t: %lx bytes\n", padding_type);

	padding_type = (unsigned long)(&lispvalue_array[0].count) -
	               (unsigned long)(&lispvalue_array[0].symbol);
	printf("(char *) symbol padded to \t: %lx bytes\n", padding_type);

	padding_type = (unsigned long)(&lispvalue_array[0].cells) -
	               (unsigned long)(&lispvalue_array[0].count);
	printf("(int) count padded to \t\t: %lx bytes\n", padding_type);

	padding_type = (unsigned long)(&lispvalue_array[1].type) -
	               (unsigned long)(&lispvalue_array[0].cells);
	printf("(LispValue **) cells padded to \t: %lx bytes\n", padding_type);

	puts("\nLispValueReordered lispvalue_reordered");
	LispValueReordered lispvalue_reordered[2];

	PRINT_PRETTY_AT("LispValueReordered   \t", lispvalue_reordered[0]);
	PRINT_PRETTY_AT("\t(int) type           ", (lispvalue_reordered[0].type));
	PRINT_PRETTY_AT("\t(int) count          ", (lispvalue_reordered[0].count));
	PRINT_PRETTY_AT("\t(double) number      ", (lispvalue_reordered[0].number));
	PRINT_PRETTY_AT("\t(char *) error       ", (lispvalue_reordered[0].error));
	PRINT_PRETTY_AT("\t(char *) symbol      ", (lispvalue_reordered[0].symbol));
	PRINT_PRETTY_AT("\t(LispValue **) cells ", (lispvalue_reordered[0].cells));

	padding_type = (unsigned long)(&lispvalue_reordered[0].count) -
	               (unsigned long)(&lispvalue_reordered[0].type);
	printf("\n(int) type padded to \t\t: %lx bytes\n", padding_type);

	padding_type = (unsigned long)(&lispvalue_reordered[0].number) -
	               (unsigned long)(&lispvalue_reordered[0].count);
	printf("(double) number padded to \t: %lx bytes\n", padding_type);

	padding_type = (unsigned long)(&lispvalue_reordered[0].error) -
	               (unsigned long)(&lispvalue_reordered[0].number);
	printf("(char *) error padded to \t: %lx bytes\n", padding_type);

	padding_type = (unsigned long)(&lispvalue_reordered[0].symbol) -
	               (unsigned long)(&lispvalue_reordered[0].error);
	printf("(char *) symbol padded to \t: %lx bytes\n", padding_type);

	padding_type = (unsigned long)(&lispvalue_reordered[0].cells) -
	               (unsigned long)(&lispvalue_reordered[0].symbol);
	printf("(int) count padded to \t\t: %lx bytes\n", padding_type);

	padding_type = (unsigned long)(&lispvalue_reordered[1].type) -
	               (unsigned long)(&lispvalue_reordered[0].cells);
	printf("(LispValue **) cells padded to \t: %lx bytes\n", padding_type);

	return 0;
}
