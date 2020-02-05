#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

static int sum;

char *xmalloc(int size)
{
	char *ret;
	sum += size;
	ret = malloc(size);
	if (!ret) {
		fprintf(stderr,"out of memory\n");
		exit(1);
	}
	memset(ret, 1, size);
	return ret;
}

int main(int argc, char *argv[])
{
	(void)argc;
	int n = atoi(argv[1]);
	int i;
	
	for (i=0;i<n;i++) {
		xmalloc(56);
		xmalloc(9);
		printf("allocated %d bytes\n", sum);
	}

}