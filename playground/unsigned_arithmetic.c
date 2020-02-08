#include <stdio.h>
#include <stdlib.h>

#define typename(_x) _Generic((_x), \
    int:     "int", \
    size_t:     "size_t", \
    float:   "float", \
    default: "other")


int main(void)
{
	size_t a, b, c;
	a = 3;
	b = a;
	c = a - b;

	printf("a : (%s)%lu\n",typename(a), a);
	printf("b : (%s)%lu\n",typename(b), b);
	printf("c : (%s)%lu\n",typename(c), c);

}