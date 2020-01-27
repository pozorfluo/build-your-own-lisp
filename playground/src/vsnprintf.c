#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

void hello_vsnprintf(const char *format, ...)
{
	va_list va_messages;
	va_start(va_messages, format);

	char *error = malloc(512);
	vsnprintf(error, 511, format, va_messages);

	error = realloc(error, strlen(error)+1);
	printf("%s", error);
	// printf("%s", va_arg(va_messages, char*));
	// printf("%s", va_arg(va_messages, char*));

	va_end(va_messages);
	free(error);
}

//--------------------------------------------------------------------- MAIN ---
int main(void)
{
	hello_vsnprintf("og message\n %s %s", "var arg0\n", "more var arg\n");
	return 0;
}