#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//------------------------------------------------------------ MAGIC NUMBERS ---
#define N_TIMES 3

int* get_random_ints(void) 
{
    // srand( (unsigned)time(NULL) );

    int random_ints[N_TIMES];

    /* dodge compiler error for educational purpose */
    /* error: function returns address of local variable */
    int *p = &random_ints[0];

    for (int i = 0; i < N_TIMES; i++)
    {
        random_ints[i] = rand();
        printf("random_ints[%d] : %d\n", i, random_ints[i]);
    }

    return p;
}

void try_block_scope(void)
{
  volatile int *ptr;
  volatile int *ptrd;

  { // Block
    int tmp = 17;
    ptr = &tmp; // Just to see if the memory is cleared
  }

  { // Sibling block
    int d = 5;
    ptrd = &d;
  }

  { // Sibling block
    int d = 7;
    ptrd = &d;
  }

  printf("\nptr = %d %d\n", *ptr, *ptrd);
  printf(
      "\naddress of pointees (void*)&(*ptr)\n"
      "\t%p %p\n", (void*)&(*ptr), (void*)&(*ptrd));

  printf(
      "\naddress of pointees (void*)ptr\n"
      "\t%p %p\n", (void*)ptr, (void*)ptrd);

  printf(
      "\naddress of pointers (void*)&ptr\n"
      "\t%p %p\n", (void*)&ptr, (void*)&ptrd);
}

int main()
{
    int *p;

    printf(
        "\n"
        "p points to an array of random int created inside a function scope"
        "\n\n");

    p = get_random_ints();


    for (int i = 0; i < N_TIMES; i++)
    {
        printf("*(p + %d)       : %d @ %p\n", i, *(p + i), (void*)&(*(p + i)));
    }
    
    printf(
        "\n"
        "run that function again but don't update what p points to"
        "\n\n");

    get_random_ints();
    get_random_ints();

    printf(
        "\n"
        "see what's wrong ?"
        "\n\n");

    for (int i = 0; i < N_TIMES; i++)
    {
        printf("*(p + %d)       : %d\n", i, *(p + i));
    }

    try_block_scope();

    return 0;
}
