#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//--------------------------------------------------------------------- MAIN ---
int main(void)
{
	int count          = 3;
	int **int_ptr_list = malloc(sizeof(int *) * count);
	int *int_ptr1      = malloc(sizeof(int));
	int *int_ptr2      = malloc(sizeof(int));
	int *int_ptr3      = malloc(sizeof(int));

	*int_ptr1           = 1;
	*int_ptr2           = 2;
	*int_ptr3           = 3;
	*int_ptr_list       = int_ptr1;
	*(int_ptr_list + 1) = int_ptr2;
	*(int_ptr_list + 2) = int_ptr3;

	printf("\n int_ptr1\t%p\n", (void *)int_ptr1);
	printf("\n *int_ptr1\t%d\n", *int_ptr1);
	printf("\n int_ptr2\t%p\n", (void *)int_ptr2);
	printf("\n *int_ptr2\t%d\n", *int_ptr2);
	printf("\n int_ptr3\t%p\n", (void *)int_ptr3);
	printf("\n *int_ptr3\t%d\n", *int_ptr3);

	printf("\n *int_ptr_list     \t%p\n", (void *)*int_ptr_list);
	printf("\n **int_ptr_list    \t%d\n", **int_ptr_list);
	printf("\n *(int_ptr_list+1) \t%p\n", (void *)*(int_ptr_list + 1));
	printf("\n **(int_ptr_list+1)\t%d\n", **(int_ptr_list + 1));
	printf("\n *(int_ptr_list+2) \t%p\n", (void *)*(int_ptr_list + 2));
	printf("\n **(int_ptr_list+2)\t%d\n", **(int_ptr_list + 2));

	memmove(int_ptr_list, (int_ptr_list + 1), sizeof(int *) * (count - 1));

	puts("\nafter memmove :");
	printf("\n int_ptr1\t%p\n", (void *)int_ptr1);
	printf("\n *int_ptr1\t%d\n", *int_ptr1);
	printf("\n int_ptr2\t%p\n", (void *)int_ptr2);
	printf("\n *int_ptr2\t%d\n", *int_ptr2);
	printf("\n int_ptr3\t%p\n", (void *)int_ptr3);
	printf("\n *int_ptr3\t%d\n", *int_ptr3);

	printf("\n *int_ptr_list     \t%p\n", (void *)*int_ptr_list);
	printf("\n **int_ptr_list    \t%d\n", **int_ptr_list);
	printf("\n *(int_ptr_list+1) \t%p\n", (void *)*(int_ptr_list + 1));
	printf("\n **(int_ptr_list+1)\t%d\n", **(int_ptr_list + 1));
	printf("\n *(int_ptr_list+2) \t%p\n", (void *)*(int_ptr_list + 2));
	printf("\n **(int_ptr_list+2)\t%d\n", **(int_ptr_list + 2));

	int_ptr_list = realloc(int_ptr_list, sizeof(int *) * (count - 1));

	puts("\nafter realloc :");
	printf("\n int_ptr1\t%p\n", (void *)int_ptr1);
	printf("\n *int_ptr1\t%d\n", *int_ptr1);
	printf("\n int_ptr2\t%p\n", (void *)int_ptr2);
	printf("\n *int_ptr2\t%d\n", *int_ptr2);
	printf("\n int_ptr3\t%p\n", (void *)int_ptr3);
	printf("\n *int_ptr3\t%d\n", *int_ptr3);

	printf("\n *int_ptr_list     \t%p\n", (void *)*int_ptr_list);
	printf("\n **int_ptr_list    \t%d\n", **int_ptr_list);
	printf("\n *(int_ptr_list+1) \t%p\n", (void *)*(int_ptr_list + 1));
	printf("\n **(int_ptr_list+1)\t%d\n", **(int_ptr_list + 1));
	// printf("\n *(int_ptr_list+2) \t%p\n", (void *)*(int_ptr_list + 2));
	// printf("\n **(int_ptr_list+2)\t%d\n", **(int_ptr_list + 2));
	free(int_ptr_list);
	free(int_ptr1);
	free(int_ptr2);
	free(int_ptr3);

	return 0;
}