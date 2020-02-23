#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
/**
 * see : http://www.catb.org/esr/structure-packing/
 */
//------------------------------------------------------------------- MACROS ---

#define PRINT_PRETTY_SIZE(_msg, _expression)                                   \
	printf(_msg "\t: %lu \tbytes %s\n",                                        \
	       sizeof(_expression),                                                \
	       pretty_byte - sizeof(_expression));

#define PRINT_PRETTY_AT(_msg, _var)                                            \
	printf(_msg "\t: %lu \tbytes\t@ %p %s\n",                                  \
	       sizeof(_var),                                                       \
	       (void *)&_var,                                                      \
	       pretty_byte - sizeof(_var));

#define PRINT_PRETTY_OFFSET(_struct, _member)                                  \
	printf(#_struct "->" #_member "    \t offset by %lu \tbytes\n",            \
	       __builtin_offsetof(_struct, _member));

//----------------------------------------------------------------- TYPEDEFS ---
typedef struct LispValue {
	int type;
	bool mutable;

	double number;
	char *error;
	char *symbol;
	/* sexpr */
	int count;
	struct LispValue **cells;
} LispValue;

//--------------------------------------------------------------------- MAIN ---
int main()
{
	/* string literal in read-only memory section */
	char *padding =
	    "################################################################";
	char *pretty_byte = padding + strlen(padding);

	puts("info about struct LispValue");
	// puts(padding);

	size_t total_unpadded_size = sizeof(int) + sizeof(bool) + sizeof(double) +
	                             sizeof(char *) + sizeof(char *) + sizeof(int) +
	                             sizeof(LispValue **);

	PRINT_PRETTY_SIZE("struct LispValue     \t", LispValue);
	printf("\ttotal_unpadded_size \t: %lu \tbytes %s\n",
	       total_unpadded_size,
	       pretty_byte - total_unpadded_size);
	PRINT_PRETTY_SIZE("\t(int) type           ", int);
	PRINT_PRETTY_SIZE("\t(bool) mutable       ", bool);
	PRINT_PRETTY_SIZE("\t(double) number      ", double);
	PRINT_PRETTY_SIZE("\t(char *) error       ", char *);
	PRINT_PRETTY_SIZE("\t(char *) symbol      ", char *);
	PRINT_PRETTY_SIZE("\t(int) count          ", int);
	PRINT_PRETTY_SIZE("\t(LispValue **) cells ", LispValue **);

	puts("\nLispValue lispvalue");
	LispValue lispvalue_array[2];

	PRINT_PRETTY_AT("LispValue            \t", lispvalue_array[0]);
	PRINT_PRETTY_AT("\t(int) type           ", (lispvalue_array[0].type));
	PRINT_PRETTY_AT("\t(bool) mutable       ", (lispvalue_array[0].mutable));
	PRINT_PRETTY_AT("\t(double) number      ", (lispvalue_array[0].number));
	PRINT_PRETTY_AT("\t(char *) error       ", (lispvalue_array[0].error));
	PRINT_PRETTY_AT("\t(char *) symbol      ", (lispvalue_array[0].symbol));
	PRINT_PRETTY_AT("\t(int) count          ", (lispvalue_array[0].count));
	PRINT_PRETTY_AT("\t(LispValue **) cells ", (lispvalue_array[0].cells));

	PRINT_PRETTY_OFFSET(LispValue, type);
	PRINT_PRETTY_OFFSET(LispValue, mutable);
	PRINT_PRETTY_OFFSET(LispValue, number);
	PRINT_PRETTY_OFFSET(LispValue, error);
	PRINT_PRETTY_OFFSET(LispValue, symbol);
	PRINT_PRETTY_OFFSET(LispValue, count);
	PRINT_PRETTY_OFFSET(LispValue, cells);

	puts("\nLispValue lispvalue");
	unsigned long padding_type;

	padding_type = (unsigned long)(&lispvalue_array[0].mutable) -
	               (unsigned long)(&lispvalue_array[0].type);
	printf("\n(bool) type padded to \t\t: %lx bytes\n", padding_type);

	padding_type = (unsigned long)(&lispvalue_array[0].number) -
	               (unsigned long)(&lispvalue_array[0].mutable);
	printf("(int) type padded to \t\t: %lx bytes\n", padding_type);

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

	puts(
	    "\nfirst address following struct data has same alignment as the "
	    "struct\n");

	return 0;
}
