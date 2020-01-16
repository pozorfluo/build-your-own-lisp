#include <stdio.h>
#include <stdlib.h>
// #include <linux/kernel.h>

//------------------------------------------------------------- DEBUG MACROS ---
#define PRINT_ADDRESS(variable)                                                \
    printf("%p <- " #variable "\n", (void *)&variable);

//------------------------------------------------------------------- MACROS ---
#define ARRAY_LENGTH(array) (sizeof(array) / sizeof((array)[0]))

//---------------------------------------------------------------- TYPEDEFFS ---
typedef int vs256i __attribute__((vector_size(sizeof(int) * 256)));
typedef int vs64i __attribute__((vector_size(sizeof(int) * 64)));
typedef int vs16i __attribute__((vector_size(sizeof(int) * 16)));

typedef float v4sf __attribute__((vector_size(sizeof(float) * 4)));

union f4vector {
    v4sf  v;
    float f[4];
};

//--------------------------------------------------------------- playground ---
int aa[256], bb[256], cc[256];

void some_loop()
{
    for (int ii = 0; ii < 256; ii++) {
	bb[ii] = ii;
	cc[ii] = ii + 2;
	aa[ii] = bb[ii] + cc[ii];
    }
}

//----------------------------------------------------------------- Function ---
/**
	* pretty prints given vs256i vector
	*
	*   -> Nothing
	*
	*/
void print_vs256i(vs256i *vector)
{
    int i = 0;

    do {
	printf("%d\t", (*vector)[i]);

	if (!(++i % 16))
	    putchar('\n');

    } while (i < 256);
}

//----------------------------------------------------------------- Function ---
/**
	* pretty prints given vs64i vector
	*
	*   -> Nothing
	*
	*/
void print_vs64i(vs64i *vector)
{
    int i = 0;

    do {
	printf("%d\t", (*vector)[i]);
	if (!(++i % 8)) {
	    putchar('\n');
	}

    } while (i < 64);
}

//--------------------------------------------------------------------- MAIN ---
/**
	*
	*   -> Error code
	*/
int main()
{
    //------------------------------------------------------- array length

    int array[] = {5, 4, 3, 2, 1, 0};

    int *pointer = array;

    PRINT_ADDRESS(array);
    PRINT_ADDRESS(array[0]);
    PRINT_ADDRESS(array[1]);
    PRINT_ADDRESS(*pointer);
    PRINT_ADDRESS(*(pointer + 1));

    printf("array_length : %zu\n", ARRAY_LENGTH(array));

    puts(
	"\nIt doesn't work for an array value that has decayed to a "
	"pointer");
    printf("array_length : %zu\n", ARRAY_LENGTH(pointer));

    puts("\nThe difference between array and &array");
    printf(
	"%p <- array"
	"\n",
	(void *)array);
    printf(
	"%p <- array"
	"\n",
	(void *)&array);
    printf(
	"%p <- array + 1"
	"\n",
	(void *)(array + 1));
    printf(
	"%p <- &array + 1"
	"\n",
	(void *)(&array + 1));

    size_t n = *(&array + 1) - array;
    printf("%zu\n", n);

    n = (&array)[1] - array;
    printf("%zu\n", n);

    puts("\n sizeof is evaluated at compile time !");

    //---------------------------------------------------------- vector
    vs256i a, b, mask, result;

    int scalar_int = 256;

    for (int i = 0; i < 256; i++) {
	a[i] = i;
    }

    b = a + scalar_int;

    puts("\na");
    print_vs256i(&a);

    puts("\nb");
    print_vs256i(&b);

    mask = a * 4;

    puts("\nmask");
    print_vs256i(&mask);

    result = __builtin_shuffle(a, b, mask);
    // result *= scalar_int;
    puts("\nresult");
    print_vs256i(&result);

    //----------------------------------------------- compressed row storage
    int row_ptr[]    = {0, 4, 7, 10, 12, 14, 14, 15, 18};
    int col_index[]  = {0, 1, 4, 6, 1, 2, 3, 2, 4, 6, 3, 4, 5, 6, 5, 0, 4, 7};
    int nnz_values[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};

    int row_count = ARRAY_LENGTH(row_ptr) - 1;
    int col_count = col_index[0];

    for (size_t i = 1; i < ARRAY_LENGTH(col_index); i++) {
	if (col_index[i] > col_count) {
	    col_count = col_index[i];
	}
    }
    col_count++;

    vs64i dense_matrix = {0};
    vs64i dense_mask   = {0};
    vs64i nnz	  = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};

    int visual_help[8][8] = {0};
    // can I use the row_ptr as a mask ?
    int offset = 0;

    for (int i = 1; i <= row_count; i++) {
	for (int j = row_ptr[i - 1]; j < row_ptr[i]; j++) {
	    visual_help[i - 1][col_index[j]]    = nnz_values[j];
	    dense_matrix[col_index[j] + offset] = nnz_values[j];
	    dense_mask[col_index[j] + offset]   = j + 1;
	}
	offset += col_count;
    }

    puts("\nsparse to padded dense");
    for (int i = 0; i < 8; i++) {
	for (int j = 0; j < 8; j++) {
	    printf("%d\t", visual_help[i][j]);
	}

	putchar('\n');
    }

    puts("\nvs64i dense_mask");
    print_vs64i(&dense_mask);

    puts("\nvs64i dense_matrix");
    print_vs64i(&dense_matrix);

    vs64i difference = (dense_matrix == __builtin_shuffle(nnz, dense_mask));

    puts("\nvs64i difference ( shuffled nnz vs dense matrix )");
    print_vs64i(&difference);

    //--------------------------------------------------------- float vector

    some_loop();

    printf("\n%d\n", aa[0]);

    union f4vector aaa, bbb, ccc;

    aaa.f[0] = 1;
    aaa.f[1] = 2;
    aaa.f[2] = 3;
    aaa.f[3] = 4;
    bbb.f[0] = 5;
    bbb.f[1] = 6;
    bbb.f[2] = 7;
    bbb.f[3] = 8;

    ccc.v = aaa.v + bbb.v;

    printf("%f, %f, %f, %f\n", ccc.f[0], ccc.f[1], ccc.f[2], ccc.f[3]);

    float scalar = 4.2;

    ccc.v = aaa.v * scalar;

    printf("%f, %f, %f, %f\n", ccc.f[0], ccc.f[1], ccc.f[2], ccc.f[3]);

    return 0;
}