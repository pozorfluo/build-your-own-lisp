#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
/**
 * see : http://www.catb.org/esr/structure-packing/
 */
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

typedef struct LispValueReordered {
	int type;
	int count;
	double number;
	char *error;
	char *symbol;
	bool mutable;
	struct LispValue **cells;
} LispValueReordered;

typedef struct fooA {
	char *p;
	char c;
	long x;
} fooA;

typedef struct fooB {
	char *p;
	char c;
} fooB;

typedef struct fooC {
	char c;
	struct fooC_inner {
		char *p;
		short x;
	} inner;
} fooC;

typedef struct foobit {
	short s;
	char c;
	int flip : 1;
	int nybble : 4;
	int septet : 7;
} foobit;

typedef struct fooD {
	int32_t i;
	int32_t i2;
	char octet[8];
	int32_t i3;
	int32_t i4;
	int64_t l;
	int32_t i5;
	int32_t i6;
} fooD;

typedef struct fooE {
	char c;
	int64_t l;
	int32_t i;
	int32_t i2;
	char sextet[6];
	int32_t i3;
	int32_t i4;
	int32_t i5;
	int32_t i6;
	char c2;
} fooE;

typedef struct fooF {
	int64_t l;
	int32_t i;
	int32_t i2;
	int32_t i3;
	int32_t i4;
	int32_t i5;
	int32_t i6;
	char c;
	char c2;
	char c3;
	char c4;
	char c5;
	char c6;
	char c7;
	char c8;
} fooF;
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

	puts("\nLispValue lispvalue");
	unsigned long padding_type;

	padding_type = (unsigned long)(&lispvalue_array[0].mutable) -
	               (unsigned long)(&lispvalue_array[0].type);
	printf("\n(bool) type padded to \t\t: %lx bytes\n", padding_type);

	padding_type = (unsigned long)(&lispvalue_array[0].number) -
	               (unsigned long)(&lispvalue_array[0].mutable);
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
	PRINT_PRETTY_AT("\t(bool) mutable       ", (lispvalue_reordered[0].mutable));
	PRINT_PRETTY_AT("\t(LispValue **) cells ", (lispvalue_reordered[0].cells));

	/**
	 * todo
	 * - [ ] fix order padding substractions
	 */
	// padding_type = (unsigned long)(&lispvalue_reordered[0].count) -
	//                (unsigned long)(&lispvalue_reordered[0].type);
	// printf("\n(int) type padded to \t\t: %lx bytes\n", padding_type);

	// padding_type = (unsigned long)(&lispvalue_reordered[0].cells) -
	//                (unsigned long)(&lispvalue_reordered[0].symbol);
	// printf("(int) count padded to \t\t: %lx bytes\n", padding_type);

	// padding_type = (unsigned long)(&lispvalue_reordered[0].number) -
	//                (unsigned long)(&lispvalue_reordered[0].count);
	// printf("(double) number padded to \t: %lx bytes\n", padding_type);

	// padding_type = (unsigned long)(&lispvalue_reordered[0].error) -
	//                (unsigned long)(&lispvalue_reordered[0].number);
	// printf("(char *) error padded to \t: %lx bytes\n", padding_type);

	// padding_type = (unsigned long)(&lispvalue_reordered[0].symbol) -
	//                (unsigned long)(&lispvalue_reordered[0].error);
	// printf("(char *) symbol padded to \t: %lx bytes\n", padding_type);

	// padding_type = (unsigned long)(&lispvalue_reordered[1].type) -
	//                (unsigned long)(&lispvalue_reordered[0].cells);
	// printf("(LispValue **) cells padded to \t: %lx bytes\n\n", padding_type);

	printf("offset of : %lu\n", ((size_t) & (((LispValue *)0)->type)));

	printf("offset of : %lu\n",
	       ((size_t)((char *)&((LispValue *)0)->type - (char *)0)));

	printf("offset of : %lu\n\n", __builtin_offsetof(LispValue, type));

	PRINT_PRETTY_OFFSET(LispValue, type);
	PRINT_PRETTY_OFFSET(LispValue, number);
	PRINT_PRETTY_OFFSET(LispValue, error);
	PRINT_PRETTY_OFFSET(LispValue, symbol);
	PRINT_PRETTY_OFFSET(LispValue, count);
	PRINT_PRETTY_OFFSET(LispValue, cells);

	PRINT_PRETTY_OFFSET(LispValueReordered, type);
	PRINT_PRETTY_OFFSET(LispValueReordered, count);
	PRINT_PRETTY_OFFSET(LispValueReordered, number);
	PRINT_PRETTY_OFFSET(LispValueReordered, error);
	PRINT_PRETTY_OFFSET(LispValueReordered, symbol);
	PRINT_PRETTY_OFFSET(LispValueReordered, cells);

	PRINT_PRETTY_OFFSET(fooA, p);
	PRINT_PRETTY_OFFSET(fooA, c);
	PRINT_PRETTY_OFFSET(fooA, x);

	puts(
	    "\nfirst address following struct data has same alignment as the "
	    "struct\n");
	PRINT_PRETTY_OFFSET(fooB, p);
	PRINT_PRETTY_OFFSET(fooB, c);
	PRINT_PRETTY_SIZE("\nstruct fooB     \t", fooB);
	PRINT_PRETTY_SIZE("\t->p     \t", char *);
	PRINT_PRETTY_SIZE("\t->c     \t", char);

	puts("7 bytes of padding after ->c\n");

	PRINT_PRETTY_OFFSET(fooC, c);
	PRINT_PRETTY_OFFSET(fooC, inner);
	PRINT_PRETTY_OFFSET(fooC, inner.p);
	PRINT_PRETTY_OFFSET(fooC, inner.x);

	PRINT_PRETTY_SIZE("\nstruct fooC     \t", fooC);
	PRINT_PRETTY_SIZE("\t->c       \t", char);
	PRINT_PRETTY_SIZE("\t->inner   \t", struct fooC_inner);
	PRINT_PRETTY_SIZE("\t->inner.p \t", char *);
	PRINT_PRETTY_SIZE("\t->inner.x \t", short);

	PRINT_PRETTY_OFFSET(foobit, s);
	PRINT_PRETTY_OFFSET(foobit, c);
	// PRINT_PRETTY_OFFSET(foobit, flip);
	// PRINT_PRETTY_OFFSET(foobit, nybble);
	// PRINT_PRETTY_OFFSET(foobit, septet);
	PRINT_PRETTY_SIZE("\nstruct foobit     \t", foobit);
	PRINT_PRETTY_SIZE("\t->s       \t", short);
	PRINT_PRETTY_SIZE("\t->c       \t", char);

	PRINT_PRETTY_SIZE("\nstruct fooD     \t", fooD);
	PRINT_PRETTY_OFFSET(fooD, i);
	PRINT_PRETTY_OFFSET(fooD, i2);
	PRINT_PRETTY_OFFSET(fooD, octet);
	PRINT_PRETTY_OFFSET(fooD, i3);
	PRINT_PRETTY_OFFSET(fooD, i4);
	PRINT_PRETTY_OFFSET(fooD, l);
	PRINT_PRETTY_OFFSET(fooD, i5);
	PRINT_PRETTY_OFFSET(fooD, i6);

	PRINT_PRETTY_SIZE("\nstruct fooE     \t", fooE);
	PRINT_PRETTY_OFFSET(fooE, c);
	PRINT_PRETTY_OFFSET(fooE, l);
	PRINT_PRETTY_OFFSET(fooE, i);
	PRINT_PRETTY_OFFSET(fooE, i2);
	PRINT_PRETTY_OFFSET(fooE, sextet);
	PRINT_PRETTY_OFFSET(fooE, i3);
	PRINT_PRETTY_OFFSET(fooE, i4);
	PRINT_PRETTY_OFFSET(fooE, i5);
	PRINT_PRETTY_OFFSET(fooE, i6);
	PRINT_PRETTY_OFFSET(fooE, c2);

	PRINT_PRETTY_SIZE("\nstruct fooF     \t", fooF);
	PRINT_PRETTY_OFFSET(fooF, l);
	PRINT_PRETTY_OFFSET(fooF, i);
	PRINT_PRETTY_OFFSET(fooF, i2);
	PRINT_PRETTY_OFFSET(fooF, i3);
	PRINT_PRETTY_OFFSET(fooF, i4);
	PRINT_PRETTY_OFFSET(fooF, i5);
	PRINT_PRETTY_OFFSET(fooF, i6);
	PRINT_PRETTY_OFFSET(fooF, c);
	PRINT_PRETTY_OFFSET(fooF, c2);
	PRINT_PRETTY_OFFSET(fooF, c3);
	PRINT_PRETTY_OFFSET(fooF, c4);
	PRINT_PRETTY_OFFSET(fooF, c5);
	PRINT_PRETTY_OFFSET(fooF, c6);
	PRINT_PRETTY_OFFSET(fooF, c7);
	PRINT_PRETTY_OFFSET(fooF, c8);

	return 0;
}
