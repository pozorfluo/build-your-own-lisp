#include <stdio.h>
#include <sys/resource.h>
#include <inttypes.h>

//--------------------------------------------------------------------- MAIN ---
int main(void)
{
	struct rlimit stack_limit;

	getrlimit(RLIMIT_STACK, &stack_limit);

	printf("soft limit : %ju bytes\n", (uintmax_t)stack_limit.rlim_cur);
	printf("hard limit : %ju bytes\n", (uintmax_t)stack_limit.rlim_max);
	printf("see also this command :\n\t$ ulimit -a\n");
	return 0;
}