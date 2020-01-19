#include <stdio.h>
#include <string.h>

//----------------------------------------------------------------- Function ---
char *pop_char(char *string, int index)
{
	char *extracted_char = &string[index];

	printf(
	    "extracted_char  before : %c @ %p\n"
	    "string[index])  before : %c @ %p\n",
	    *extracted_char,
	    (void *)extracted_char,
	    string[index],
	    (void *)&string[index]);

	memmove(
	    &string[index], &string[index + 1], (size_t)(strlen(string) - index));

	printf(
	    "extracted_char   after : %c @ %p\n"
	    "string[index])   after : %c @ %p\n",
	    *extracted_char,
	    (void *)extracted_char,
	    string[index],
	    (void *)&string[index]);

	return extracted_char;
}

//--------------------------------------------------------------------- MAIN ---
int main(void)
{
	char destination[]     = "destination";
	const char source[]    = "source";
	char *destination_ptr  = "destination";
	const char *source_ptr = "source";

	printf("destination[] : %zu\n",
	       (sizeof(destination) / sizeof((destination)[0])));

	printf("source[] : %zu\n", (sizeof(source) / sizeof((source)[0])));

	printf("destination[] : %zu\n", strlen(destination_ptr));

	printf("source[] : %zu\n", strlen(source));

	/* bad juju */

	// printf(
	// 	"*destination_ptr : %zu\n",
	// 	(sizeof(destination_ptr) / sizeof((destination_ptr)[0])));

	// printf(
	// 	"*source_ptr : %zu\n",
	// 	(sizeof(source_ptr) / sizeof((source_ptr)[0])));

	/* they disagree because strlen doesn't count the terminating null char */
	printf("destination_ptr : %zu\n", strlen(destination_ptr));
	printf("source_ptr : %zu\n", strlen(source_ptr));

	printf(
	    "\n"
	    "before memmove\n"
	    "destination = %s\n"
	    "source = %s\n",
	    destination,
	    source);

	size_t starting_length = strlen(destination) - 1;
	for (size_t i = 0; i <= starting_length; i++) {
		printf("destination before : %s\n", destination);
		printf("extracted          : %s\n", pop_char(destination, 0));
		printf("destination after  : %s\n\n", destination);
	}

	// for (size_t i = 1; i <= (strlen(source) + 1); i++) {
	// 	memmove(destination + 2, source, i);

	// 	printf(
	// 		"\n"
	// 		"after memmove\n"
	// 		"destination = %s\n"
	// 		"source = %s\n",
	// 		destination,
	// 		source);
	// }

	// size_t starting_length = strlen(destination) + 1;
	// for (size_t i = 0; i <= starting_length; i++) {
	// 	char fresh_destination[] = "destination";
	// 	memmove(fresh_destination + i, source, (strlen(source) + 1));

	// 	printf(
	// 		"\n"
	// 		"after memmove\n"
	// 		"fresh_destination = %s\n"
	// 		"source = %s\n",
	// 		fresh_destination,
	// 		source);
	// }

	// for (size_t i = 1; i <= (strlen(source) + 1); i++) {
	// 	memmove(destination + 2, source, i);

	// 	printf(
	// 		"\n"
	// 		"after memmove\n"
	// 		"destination = %s\n"
	// 		"source = %s\n",
	// 		destination,
	// 		source);
	// }

	return 0;
}
