#include <stdio.h>

//--------------------------------------------------------------------- MAIN ---
int main(void)
{
	char *error_message = "";
	char *other_identicalcontent_string = "";
	char *other_string = "function def";


	printf("error_message : %s\n", error_message);
	error_message = "can I do this";
	printf("error_message : %s\n", error_message);
	error_message =
	    "yes and you are pointing to a new string litteral in read-only memory";
	printf("error_message : %s\n", error_message);

	printf(other_string);
	return 0;
}