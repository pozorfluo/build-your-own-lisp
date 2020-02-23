#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/linenoise.h"
#include "../include/mpc.h"

//---------------------------------------------------------- PLATFORM MACROS ---

//-------------------------------------------------------------- ANSI MACROS ---
/**
 * see https://invisible-island.net/xterm/ctlseqs/ctlseqs.html
 *
 * todo
 *   [] clarify which are ready-made sequences
 *   [] clarify which are bits to be expanded into sequence
 */
#define ESC "\x1b"

#define RESET ESC "[0m"
#define BOLD ESC "[1m"
#define UNDERSCORE ESC "[4m"
#define BLINK ESC "[5m"
#define REVERSE ESC "[7m"
#define CONCEALED ESC "[8m"

#define FG "3"
#define BG "4"
#define FG_BRIGHT "9"
#define BG_BRIGHT "10"

#define BLACK "0"
#define RED "1"
#define GREEN "2"
#define YELLOW "3"
#define BLUE "4"
#define MAGENTA "5"
#define CYAN "6"
#define WHITE "7"
#define DEFAULT "9"

#define FG_BLACK ESC "[" FG BLACK "m"
#define FG_RED ESC "[" FG RED "m"
#define FG_GREEN ESC "[" FG GREEN "m"
#define FG_YELLOW ESC "[" FG YELLOW "m"
#define FG_BLUE ESC "[" FG BLUE "m"
#define FG_MAGENTA ESC "[" FG MAGENTA "m"
#define FG_CYAN ESC "[" FG CYAN "m"
#define FG_WHITE ESC "[" FG WHITE "m"
#define FG_DEFAULT ESC "[" FG DEFAULT "m"

#define BG_BLACK ESC "[" BG BLACK "m"
#define BG_RED ESC "[" BG RED "m"
#define BG_GREEN ESC "[" BG GREEN "m"
#define BG_YELLOW ESC "[" BG YELLOW "m"
#define BG_BLUE ESC "[" BG BLUE "m"
#define BG_MAGENTA ESC "[" BG MAGENTA "m"
#define BG_CYAN ESC "[" BG CYAN "m"
#define BG_WHITE ESC "[" BG WHITE "m"
#define BG_DEFAULT ESC "[" BG DEFAULT "m"

#define FG_BRIGHT_BLACK ESC "[" FG_BRIGHT BLACK "m"
#define FG_BRIGHT_RED ESC "[" FG_BRIGHT RED "m"
#define FG_BRIGHT_GREEN ESC "[" FG_BRIGHT GREEN "m"
#define FG_BRIGHT_YELLOW ESC "[" FG_BRIGHT YELLOW "m"
#define FG_BRIGHT_BLUE ESC "[" FG_BRIGHT BLUE "m"
#define FG_BRIGHT_MAGENTA ESC "[" FG_BRIGHT MAGENTA "m"
#define FG_BRIGHT_CYAN ESC "[" FG_BRIGHT CYAN "m"
#define FG_BRIGHT_WHITE ESC "[" FG_BRIGHT WHITE "m"
#define FG_BRIGHT_DEFAULT ESC "[" FG_BRIGHT DEFAULT "m"

#define BG_BRIGHT_BLACK ESC "[" BG_BRIGHT BLACK "m"
#define BG_BRIGHT_RED ESC "[" BG_BRIGHT RED "m"
#define BG_BRIGHT_GREEN ESC "[" BG_BRIGHT GREEN "m"
#define BG_BRIGHT_YELLOW ESC "[" BG_BRIGHT YELLOW "m"
#define BG_BRIGHT_BLUE ESC "[" BG_BRIGHT BLUE "m"
#define BG_BRIGHT_MAGENTA ESC "[" BG_BRIGHT MAGENTA "m"
#define BG_BRIGHT_CYAN ESC "[" BG_BRIGHT CYAN "m"
#define BG_BRIGHT_WHITE ESC "[" BG_BRIGHT WHITE "m"
#define BG_BRIGHT_DEFAULT ESC "[" BG_BRIGHT DEFAULT "m"

#define DEFAULT_COLORS ESC "[39;49m"

/* codes in the sequence must be separated by ";" */
#define ANSI(SEQUENCE) ESC "[" SEQUENCE "m"

//------------------------------------------------------------- DEBUG MACROS ---
/**
 * Helps me follow malloc / free where needed
 */
#ifdef DEBUG_MALLOC

#include <malloc.h>

void *xmalloc(size_t size, const char *origin, const char *destination)
{
	void *return_pointer = malloc(size);
	if ((return_pointer == NULL) && !size) {
		return_pointer = malloc(1);
	}
	if ((return_pointer == NULL)) {
		printf(FG_RED REVERSE
		       "malloc failed, Out of memory I guess\n"
		       "\tcalled inside      : %s()\n"
		       "\ttried to allocate  : %lu bytes\n"
		       "\tfor                : %s\n" RESET,
		       origin,
		       size,
		       destination);
		exit(EXIT_FAILURE);
	}

	printf(FG_CYAN "\tmallocing %s %lu bytes inside %s()\n" RESET,
	       destination,
	       size,
	       origin);
	memset(return_pointer, 0xFB, size);
	return return_pointer;
}

void xfree(void *pointer, const char *pointer_name, const char *origin)
{
	size_t bytes_in_malloced_block = malloc_usable_size(pointer);
	printf(FG_BLUE "\tfreeing %s %lu bytes inside %s()\n" RESET,
	       pointer_name,
	       bytes_in_malloced_block,
	       origin);
	free(pointer);
}

#define XMALLOC(_size, _origin, _destination)                                  \
	xmalloc(_size, _origin, _destination)

#define XFREE(_pointer, _origin) xfree(_pointer, #_pointer, _origin)

#else
#define XMALLOC(_size, _origin, _destination) malloc(_size)
#define XFREE(_pointer, _origin) free(_pointer)
#endif

//------------------------------------------------------------------- MACROS ---
#define ARRAY_LENGTH(array) (sizeof(array) / sizeof((array)[0]))

#define LVAL_ASSERT(_arguments, _condition, _format, ...)                      \
	if (!(_condition)) {                                                       \
		LispValue *error = new_lispvalue_error(_format, ##__VA_ARGS__);        \
		delete_lispvalue(_arguments);                                          \
		return error;                                                          \
	}

#define LVAL_ASSERT_TYPE(_function, _arguments, _element_index, _expected)     \
	LVAL_ASSERT(                                                               \
	    _arguments,                                                            \
	    _arguments->cells[_element_index]->type == _expected,                  \
	    "Function '%s' passed incorrect type for element %d !\n"               \
	    "\t expected %s, got %s.",                                             \
	    _function,                                                             \
	    _element_index,                                                        \
	    lispvalue_type_tostring(_expected),                                    \
	    lispvalue_type_tostring(_arguments->cells[_element_index]->type))

#define LVAL_ASSERT_COUNT(_function, _arguments, _expected)                    \
	LVAL_ASSERT(_arguments,                                                    \
	            _arguments->count == _expected,                                \
	            "Function '%s' passed incorrect number of arguments !\n"       \
	            "\t expected %d, got %d.",                                     \
	            _function,                                                     \
	            _expected,                                                     \
	            _arguments->count)

#define LVAL_ASSERT_NOT_EMPTY(_function, _arguments, _element_index)           \
	LVAL_ASSERT(_arguments,                                                    \
	            _arguments->cells[_element_index]->count != 0,                 \
	            "Function '%s' passed {} for element %d!\n",                   \
	            _function,                                                     \
	            _element_index)

/**
 * Suppresses compiler warning about unused parameters needed in
 * function signatures
 *
 * Debating whether to use gcc specific __attribute__((unused)) or this
 */
#define UNUSED(_parameter) (void)(_parameter)

//----------------------------------------------------- FORWARD DECLARATIONS ---
struct LispValue;
struct LispEnv;
typedef struct LispValue LispValue;
typedef struct LispEnv LispEnv;

//------------------------------------------------------------- DECLARATIONS ---
enum LispValueType {
	LVAL_ERR,
	LVAL_NUMBER,
	LVAL_SYMBOL,
	LVAL_FUNCTION,
	LVAL_SEXPR,
	LVAL_QEXPR
};

typedef LispValue *(*LispBuiltin)(LispEnv *, LispValue *);

struct LispValue {
	int type;
	bool mutable;

	double number;
	char *error;
	char *symbol;
	LispBuiltin function;
	size_t count;
	struct LispValue **cells;
};

struct LispEnv {
	/* entries in each list match their corresponding */
	/* entry in the other list by index */
	size_t count;
	char **symbols;
	LispValue **values;
};

//--------------------------------------------------------------- PROTOTYPES ---
/**
 * todo
 * - [ ] rename parameters to be more consistent overall but still locally
 *       unambiguously relevant
 */
static LispValue *
builtin_operator(LispEnv *env, LispValue *arguments, const char *operator);
static LispValue *take_lispvalue(LispValue *value, const int index);
static LispValue *pop_lispvalue(LispValue *value, const int index);
static LispValue *eval_lispvalue(LispEnv *env, LispValue *value);
static LispValue *copy_lispvalue(LispValue *value);
static void print_lispvalue(LispEnv *env, LispValue *value);

//----------------------------------------------------------------- Function ---
/**
 * Gets a readable name for a given LispValueType
 *   -> pointer to string
 */
static char *lispvalue_type_tostring(int type)
{
	switch (type) {
	case LVAL_ERR:
		return "Error";
	case LVAL_NUMBER:
		return "Number";
	case LVAL_SYMBOL:
		return "Symbol";
	case LVAL_FUNCTION:
		return "Function";
	case LVAL_SEXPR:
		return "S-Expression";
	case LVAL_QEXPR:
		return "Q-Expression";
	default:
		return "Unknown";
	}
}
//----------------------------------------------------------------- Function ---
/**
 * Creates a new LispValue of type number for a given a number
 *   -> pointer to new LispValue number
 */
static LispValue *new_lispvalue_number(const double number)
{
	LispValue *new_value =
	    XMALLOC(sizeof(LispValue), "new_lispvalue_number", "new_value");

	new_value->type   = LVAL_NUMBER;
	new_value->number = number;

	return new_value;
}

//----------------------------------------------------------------- Function ---
/**
 * Creates a new LispValue of type error for a given a error format and given
 * variable number of messages
 *
 *   Create and init a variadic argument list
 *   Allocate max intended space to new LispValue error message
 *   Print formated to error message with max allocated - 1 for terminating \0
 *   Reallocate error message to size actually used
 *   Cleanup variadic argument list
 *     -> pointer to new LispValue error
 */
static LispValue *new_lispvalue_error(const char *format, ...)
{
	va_list va_messages;
	va_start(va_messages, format);

	LispValue *new_value =
	    XMALLOC(sizeof(LispValue), "new_lispvalue_error", "new_value");
	new_value->type  = LVAL_ERR;
	new_value->error = XMALLOC(512, "new_lispvalue_error", "new_value->error");

	vsnprintf(new_value->error, 511, format, va_messages);

	new_value->error = realloc(new_value->error, strlen(new_value->error) + 1);

	va_end(va_messages);

	return new_value;
}

//----------------------------------------------------------------- Function ---
/**
 * Creates a new LispValue of type symbol for a given a symbol
 *   -> pointer to new LispValue symbol
 */
static LispValue *new_lispvalue_symbol(const char *symbol)
{
	LispValue *new_value =
	    XMALLOC(sizeof(LispValue), "new_lispvalue_symbol", "new_value");

	new_value->type   = LVAL_SYMBOL;
	new_value->symbol = XMALLOC(
	    strlen(symbol) + 1, "new_lispvalue_symbol", "new_value->symbol");
	strcpy(new_value->symbol, symbol);

	return new_value;
}

//----------------------------------------------------------------- Function ---
/**
 * Creates a new empty LispValue of type function
 *   -> pointer to new LispValue function
 */
static LispValue *new_lispvalue_function(LispBuiltin function)
{
	LispValue *new_value =
	    XMALLOC(sizeof(LispValue), "new_lispvalue_function", "new_value");

	new_value->type     = LVAL_FUNCTION;
	new_value->function = function;

	return new_value;
}

//----------------------------------------------------------------- Function ---
/**
 * Creaes a new empty LispEnvironment
 *
 *   -> pointer to new LispEnvironment
 */
static LispEnv *new_lispenv(void)
{
	LispEnv *new_env = XMALLOC(sizeof(LispEnv), "new_lispenv", "new_env");
	new_env->count   = 0;
	new_env->symbols = NULL;
	new_env->values  = NULL;
	return new_env;
}

//----------------------------------------------------------------- Function ---
/**
 * Creates a new empty LispValue of type sexpr
 *   -> pointer to new LispValue sexpr
 */

static LispValue *new_lispvalue_sexpr(void)
{
	LispValue *new_value =
	    XMALLOC(sizeof(LispValue), "new_lispvalue_sexpr", "new_value");

	new_value->type  = LVAL_SEXPR;
	new_value->count = 0;
	new_value->cells = NULL;

	return new_value;
}

//----------------------------------------------------------------- Function ---
/**
 * Creates a new empty LispValue of type qexpr
 *   -> pointer to new LispValue qexpr
 */
static LispValue *new_lispvalue_qexpr(void)
{
	LispValue *new_value =
	    XMALLOC(sizeof(LispValue), "new_lispvalue_qexpr", "new_value");

	new_value->type  = LVAL_QEXPR;
	new_value->count = 0;
	new_value->cells = NULL;

	return new_value;
}

//----------------------------------------------------------------- Function ---
/**
 * Frees given LispValue ressources according to its type
 *   -> nothing
 */
static void delete_lispvalue(LispValue *value)
{
	switch (value->type) {
	case LVAL_NUMBER:
		break;

	case LVAL_ERR:
		XFREE(value->error, "delete_lispvalue");
		break;

	case LVAL_SYMBOL:
		XFREE(value->symbol, "delete_lispvalue");
		break;

	case LVAL_FUNCTION:
		break;

	case LVAL_SEXPR:
	case LVAL_QEXPR:
		for (size_t i = 0; i < value->count; i++) {
			delete_lispvalue(value->cells[i]);
		}

		XFREE(value->cells, "delete_lispvalue");
		break;

	default:
		break;
	}

	XFREE(value, "delete_lispvalue");
}

//----------------------------------------------------------------- Function ---
/**
 * Frees given LispEnv ressources
 *   -> nothing
 */
static void delete_lispenv(LispEnv *env)
{
	for (size_t i = 0; i < env->count; i++) {
		XFREE(env->symbols[i], "delete_lispenv");
		delete_lispvalue(env->values[i]);
	}
	XFREE(env->symbols, "delete_lispenv");
	XFREE(env->values, "delete_lispenv");
	XFREE(env, "delete_lispenv");
}

//----------------------------------------------------------------- Function ---
/**
 * Gets LispValue associated with given LispValue symbol if any
 *   -> pointer to new copy of associated LispValue
 *
 * Iterate over all items in environment
 *   If stored symbol matches given Lispvalue symbol
 *     -> copy of associated value in environment
 *   -> error if no symbol match found
 *
 * todo
 *   - [ ] plug your upcoming first hash table implementation here :)
 */
static LispValue *get_lispenv(LispEnv *env, LispValue *value)
{
	for (size_t i = 0; i < env->count; i++) {
		if (strcmp(env->symbols[i], value->symbol) == 0) {
			return copy_lispvalue(env->values[i]);
		}
	}
	return new_lispvalue_error("unbound symbol '%s' !", value->symbol);
}
//----------------------------------------------------------------- Function ---
/**
 * Gets symbol associated with given LispBuiltin function if any
 *   -> pointer to symbol string
 *
 * Iterate over all value in environment
 *   If stored function pointer matches given LispBuiltin function
 *     -> pointer to associated symbol string
 *   -> pointer to error message string if no match found
 */
static char *get_symbol_lispenv(LispEnv *env, LispBuiltin function)
{
	for (size_t i = 0; i < env->count; i++) {
		if (env->values[i]->function == function) {
			return env->symbols[i];
		}
	}

	return "error : unknown function";
}

//----------------------------------------------------------------- Function ---
/**
 * Creates a new entry in given LispEnvironment for given symbol, value pair
 * or replaces value associated with existing symbol in given LispEnvironment
 * with given value
 *   -> nothing
 *
 * Iterate over all items in given LispEnv
 *   If symbol is found
 *     Delete value at that position
 *     Replace with copy of given value
 * 	     -> nothing
 * Update entry count
 * Update lists size
 * Copy contents of given value and symbol into new entry
 *   -> nothing
 */
static void put_lispenv(LispEnv *env, LispValue *symbol, LispValue *value)
{
	for (size_t i = 0; i < env->count; i++) {
		if (strcmp(env->symbols[i], symbol->symbol) == 0) {
			delete_lispvalue(env->values[i]);
			env->values[i] = copy_lispvalue(value);
			return;
		}
	}

	env->count++;
	env->symbols = realloc(env->symbols, sizeof(char *) * env->count);
	env->values  = realloc(env->values, sizeof(LispValue *) * env->count);

	env->symbols[env->count - 1] = XMALLOC(strlen(symbol->symbol) + 1,
	                                       "put_lispenv",
	                                       "env->symbols[env->count - 1]");
	strcpy(env->symbols[env->count - 1], symbol->symbol);
	env->values[env->count - 1] = copy_lispvalue(value);
}

//----------------------------------------------------------------- Function ---
/**
 * Evaluates given AST node of type number
 *   -> pointer to a LispValue number
 */
static LispValue *read_lispvalue_number(mpc_ast_t *ast)
{
	errno         = 0;
	double number = strtod(ast->contents, NULL);

	return (errno != ERANGE) ? new_lispvalue_number(number)
	                         : new_lispvalue_error("invalid number !");
}

//----------------------------------------------------------------- Function ---
/**
 * Adds a given LispValue to the list of expressions of a given LispValue
 * of type sexpr or qexpr
 *   -> pointer to given LispValue sexpr
 */
static LispValue *add_lispvalue(LispValue *expr, LispValue *value)
{
	expr->count++;
	expr->cells = realloc(expr->cells, sizeof(LispValue *) * expr->count);
	expr->cells[expr->count - 1] = value;

	return expr;
}

//----------------------------------------------------------------- Function ---
/**
 * Traverses given AST and evaluates the expression
 *   -> pointer to a LispValue
 */
static LispValue *read_lispvalue(mpc_ast_t *ast)
{
	if (strstr(ast->tag, "number")) {
		return read_lispvalue_number(ast);
	}
	else if (strstr(ast->tag, "symbol")) {
		return new_lispvalue_symbol(ast->contents);
	}
	else {
		LispValue *lispvalue = NULL;

		if (!(strcmp(ast->tag, ">")) || (strstr(ast->tag, "sexpr"))) {
			lispvalue = new_lispvalue_sexpr();
		}
		else if (strstr(ast->tag, "qexpr")) {
			lispvalue = new_lispvalue_qexpr();
		}

		for (int i = 0; i < ast->children_num; i++) {
			if (!(strcmp(ast->children[i]->contents, "(")) ||
			    !(strcmp(ast->children[i]->contents, ")")) ||
			    !(strcmp(ast->children[i]->contents, "{")) ||
			    !(strcmp(ast->children[i]->contents, "}")) ||
			    !(strcmp(ast->children[i]->tag, "regex"))) {
				continue;
			}
			else {
				lispvalue =
				    add_lispvalue(lispvalue, read_lispvalue(ast->children[i]));
			}
		}

		return lispvalue;
	}
}

//----------------------------------------------------------------- Function ---
/**
 * Creates a copy of given LispValue
 *   -> pointer to new LispValue
 */
static LispValue *copy_lispvalue(LispValue *value)
{
	LispValue *copy = XMALLOC(sizeof(LispValue), "copy_lispvalue", "copy");
	copy->type      = value->type;

	switch (value->type) {
	case LVAL_NUMBER:
		copy->number = value->number;
		break;

	case LVAL_FUNCTION:
		copy->function = value->function;
		break;

	case LVAL_ERR:
		copy->error =
		    XMALLOC(strlen(value->error) + 1, "copy_lispvalue", "copy->error");
		strcpy(copy->error, value->error);
		break;

	case LVAL_SYMBOL:
		copy->symbol = XMALLOC(
		    strlen(value->symbol) + 1, "copy_lispvalue", "copy->symbol");
		strcpy(copy->symbol, value->symbol);
		break;

	case LVAL_SEXPR:
	case LVAL_QEXPR:
		copy->count = value->count;
		copy->cells = XMALLOC(
		    sizeof(LispValue *) * copy->count, "copy_lispvalue", "copy->cells");
		for (size_t i = 0; i < copy->count; i++) {
			copy->cells[i] = copy_lispvalue(value->cells[i]);
		}

	default:
		break;
	}

	return copy;
}

//----------------------------------------------------------------- Function ---
/**
 * Pretty prints given LispValue of type sexpr
 *   -> Nothing
 */
static void print_lispvalue_expr(LispEnv *env,
                                 LispValue *value,
                                 const char open,
                                 const char close)
{
	putchar(open);

	for (size_t i = 0; i < value->count; i++) {
		print_lispvalue(env, value->cells[i]);
		putchar(' ');
	}

	putchar(close);
}

//----------------------------------------------------------------- Function ---
/**
 * Pretty prints given LispValue according to its type
 *   -> Nothing
 */
static void print_lispvalue(LispEnv *env, LispValue *value)
{
	switch (value->type) {
	case LVAL_NUMBER:
		printf(FG_YELLOW "%f" RESET, value->number);
		break;

	case LVAL_ERR:
		printf(FG_RED REVERSE "Error : " RESET FG_MAGENTA " %s" RESET,
		       value->error);
		break;

	case LVAL_SYMBOL:
		printf(FG_CYAN "%s" RESET, value->symbol);
		break;

	case LVAL_FUNCTION:
		printf(FG_GREEN "<%s>" RESET, get_symbol_lispenv(env, value->function));
		break;

	case LVAL_SEXPR:
		print_lispvalue_expr(env, value, '(', ')');
		break;

	case LVAL_QEXPR:
		print_lispvalue_expr(env, value, '{', '}');
		break;

	default:
		break;
	}
}

//----------------------------------------------------------------- Function ---
/**
 * Pretty prints given LispValue followed by newline
 *   -> Nothing
 */
static void print_lispvalue_newline(LispEnv *env, LispValue *value)
{
	print_lispvalue(env, value);
	putchar('\n');
}

//----------------------------------------------------------------- Function ---
/**
 * Evaluates given LispValue of type sexpr
 *   -> pointer to result LispValue
 */
static LispValue *eval_lispvalue_sexpr(LispEnv *env, LispValue *value)
{
	/* evaluate children */
	for (size_t i = 0; i < value->count; i++) {
		value->cells[i] = eval_lispvalue(env, value->cells[i]);
	}

	/* check for errors */
	for (size_t i = 0; i < value->count; i++) {
		if (value->cells[i]->type == LVAL_ERR) {
			return take_lispvalue(value, i);
		}
	}

	/* empty expression */
	if (value->count == 0) {
		// printf("empty expression ");
		return value;
	}
	/* single expression */
	else if (value->count == 1) {
		// printf("single expression ");
		return take_lispvalue(value, 0);
	}
	else {
		/* make sure first element is a function after evaluation */
		LispValue *first_element = pop_lispvalue(value, 0);

		if (first_element->type != LVAL_FUNCTION) {
			LispValue *error = new_lispvalue_error(
			    "S-Expressions invalid first element type !\n"
			    "\t expected %s, got %s.",
			    lispvalue_type_tostring(LVAL_FUNCTION),
			    lispvalue_type_tostring(first_element->type));

			delete_lispvalue(first_element);
			delete_lispvalue(value);
			return error;
		}
		else {
			LispValue *result = first_element->function(env, value);
			delete_lispvalue(first_element);
			return result;
		}
	}
}

//----------------------------------------------------------------- Function ---
/**
 * Evaluates given LispValue
 *   -> pointer to result LispValue
 */
static LispValue *eval_lispvalue(LispEnv *env, LispValue *value)
{
	switch (value->type) {
	case LVAL_SYMBOL: {
		LispValue *retrieved_lispvalue = get_lispenv(env, value);
		delete_lispvalue(value);
		return retrieved_lispvalue;
	}

	case LVAL_SEXPR:
		return eval_lispvalue_sexpr(env, value);

	default:
		return value;
	}
}

//----------------------------------------------------------------- Function ---
/**
 * Extracts single element from given LispValue of type sexpr
 * Shifts the rest of element list pointer backward over extracted element
 * pointer
 *   -> Extracted LispValue
 */
static LispValue *pop_lispvalue(LispValue *value, const int index)
{
	LispValue *extracted_element = value->cells[index];

	memmove(&value->cells[index],
	        &value->cells[index + 1],
	        sizeof(LispValue *) * (value->count - index - 1));

	value->count--;

	value->cells = realloc(value->cells, sizeof(LispValue *) * value->count);

	return extracted_element;
}

//----------------------------------------------------------------- Function ---
/**
 * Extracts single element from given LispValue of type sexpr
 * Deletes the rest
 *   -> Extracted LispValue
 *
 * todo
 * - [ ] check your understanding of the logic behind delete_lispvalue(value)
 */
static LispValue *take_lispvalue(LispValue *value, const int index)
{
	LispValue *extracted_element = pop_lispvalue(value, index);
	delete_lispvalue(value);

	return extracted_element;
}

//----------------------------------------------------------------- Function ---
/**
 * Goes over all elements of given LispValue
 * Checks that they are all of type number
 *   -> Truth value of predicate
 *
 */
static int are_all_numbers(LispValue *arguments)
{
	// int result = 1;
	for (size_t i = 0; i < arguments->count; i++) {
		if (arguments->cells[i]->type != LVAL_NUMBER) {
			// result = 0;
			return 0;
		}
	}
	return 1;
}

//----------------------------------------------------------------- Function ---
/**
 * Calls builtin_operator with appropriate symbol characters for basic math
 * operations
 * Used to register function with given LispEnv
 *  -> Evaluation result LispValue
 */
static LispValue *builtin_add(LispEnv *env, LispValue *value)
{
	return builtin_operator(env, value, "+");
}

static LispValue *builtin_sub(LispEnv *env, LispValue *value)
{
	return builtin_operator(env, value, "-");
}

static LispValue *builtin_mul(LispEnv *env, LispValue *value)
{
	return builtin_operator(env, value, "*");
}

static LispValue *builtin_div(LispEnv *env, LispValue *value)
{
	return builtin_operator(env, value, "/");
}

static LispValue *builtin_mod(LispEnv *env, LispValue *value)
{
	return builtin_operator(env, value, "%");
}

static LispValue *builtin_pow(LispEnv *env, LispValue *value)
{
	return builtin_operator(env, value, "^");
}

static LispValue *builtin_max(LispEnv *env, LispValue *value)
{
	return builtin_operator(env, value, ">");
}

static LispValue *builtin_min(LispEnv *env, LispValue *value)
{
	return builtin_operator(env, value, "<");
}

//----------------------------------------------------------------- Function ---
/**
 * Dump given LispEnv
 * Return a Q-Expression with the list of symbols in given LispEnv
 *
 * 	 Create a new Q-Expression the size of given LispEnv
 *   Iterate over all items in given LispEnv
 * 		Print item's symbol
 * 		Print item's value
 * 		Add a copy LispValue for each item found to this Q-Expression
 *
 *     -> pointer to LispValue error or Q-Expression
 */
static LispValue *builtin_env(LispEnv *env, LispValue *arguments)
{
	LispValue *qexpr = new_lispvalue_qexpr();
	qexpr->count     = env->count;
	// this is not a good place to be caught with your pants down
	qexpr->cells = realloc(qexpr->cells, sizeof(LispValue *) * qexpr->count);

	for (size_t i = 0; i < env->count; i++) {
		printf("\t%s\t", env->symbols[i]);
		print_lispvalue(env, env->values[i]);
		putchar('\n');
		// qexpr->cells[i] = copy_lispvalue(env->values[i]);
		LispValue *copy = XMALLOC(sizeof(LispValue), "builtin_env", "copy");
		copy->type      = LVAL_SYMBOL;
		copy->symbol =
		    XMALLOC(strlen(env->symbols[i]) + 1, "builtin_env", "copy");
		strcpy(copy->symbol, env->symbols[i]);
		qexpr->cells[i] = copy;
	}

	delete_lispvalue(arguments);
	return qexpr;
}

//----------------------------------------------------------------- Function ---
/**
 * Returns a Q-Expression with only the first element of given Q-Expression
 *
 *   Ensure only a single argument is passed
 *   Ensure that argument is a Q-Expression
 *   Ensure that Q-Expression is not empty
 *   Take first element of that Q-Expression
 *   Delete all other elements of that Q-Expression
 *     -> pointer to modified Q-Expression
 */
static LispValue *builtin_head(LispEnv *env, LispValue *arguments)
{
	UNUSED(env);

	LVAL_ASSERT_COUNT("head", arguments, 1);
	LVAL_ASSERT_TYPE("head", arguments, 0, LVAL_QEXPR);
	LVAL_ASSERT_NOT_EMPTY("head", arguments, 0);

	LispValue *head = take_lispvalue(arguments, 0);
	while (head->count > 1) {
		delete_lispvalue(pop_lispvalue(head, 1));
	}

	return head;
}
//----------------------------------------------------------------- Function ---
/**
 * Returns a Q-Expression with the first element of given Q-Expression removed
 *
 *   Ensure only a single argument is passed
 *   Ensure that argument is a Q-Expression
 *   Ensure that Q-Expression is not empty
 *   Take first element of that Q-Expression
 *   Delete first element of that Q-Expression
 *     -> pointer to modified Q-Expression
 */
static LispValue *builtin_tail(LispEnv *env, LispValue *arguments)
{
	UNUSED(env);

	LVAL_ASSERT_COUNT("tail", arguments, 1);
	LVAL_ASSERT_TYPE("tail", arguments, 0, LVAL_QEXPR);
	LVAL_ASSERT_NOT_EMPTY("tail", arguments, 0);

	LispValue *tail = take_lispvalue(arguments, 0);
	delete_lispvalue(pop_lispvalue(tail, 0));

	return tail;
}
//----------------------------------------------------------------- Function ---
/**
 * Returns a Q-Expression containing given arguments
 *
 *   Convert input S-Expression to Q-Expression
 *     -> pointer to a Q-Expression
 */
static LispValue *builtin_list(LispEnv *env, LispValue *arguments)
{
	UNUSED(env);

	arguments->type = LVAL_QEXPR;
	return arguments;
}
//----------------------------------------------------------------- Function ---
/**
 * Returns a Q-Expression with the last element of given Q-Expression removed
 *
 *   Ensure only a single argument is passed
 *   Ensure that argument is a Q-Expression
 *   Ensure that Q-Expression is not empty
 *   Delete last element of that Q-Expression
 *   Update Q-Expression size
 *     -> pointer to modified Q-Expression
 */
static LispValue *builtin_init(LispEnv *env, LispValue *arguments)
{
	UNUSED(env);

	LVAL_ASSERT_COUNT("init", arguments, 1);
	LVAL_ASSERT_TYPE("init", arguments, 0, LVAL_QEXPR);
	LVAL_ASSERT_NOT_EMPTY("init", arguments, 0);

	LispValue *init = take_lispvalue(arguments, 0);

	delete_lispvalue(init->cells[init->count - 1]);
	init->count--;
	init->cells = realloc(init->cells, sizeof(LispValue *) * init->count);

	return init;
}

//----------------------------------------------------------------- Function ---
/**
 * Evaluates given Q-Expression
 *
 *   Ensure only a single argument is passed
 *   Ensure that argument is a Q-Expression
 *   Convert input Q-Expression to S-Expression
 *   Evaluate converted S-Expression
 *     -> pointer to result LispValue
 */
static LispValue *builtin_eval(LispEnv *env, LispValue *arguments)
{
	UNUSED(env);

	LVAL_ASSERT_COUNT("eval", arguments, 1);
	LVAL_ASSERT_TYPE("eval", arguments, 0, LVAL_QEXPR);

	LispValue *expression = take_lispvalue(arguments, 0);
	expression->type      = LVAL_SEXPR;

	return eval_lispvalue(env, expression);
}
//----------------------------------------------------------------- Function ---
/**
 * Joins given LispValues together
 *
 *  Pop each item from second given LispValue until it's empty and
 *  Add each item to first given LispValue
 *  Delete second given LispValue
 *   -> Return pointer to first given LispValue
 */
static LispValue *join_lispvalue(LispValue *first, LispValue *second)
{
	while (second->count) {
		first = add_lispvalue(first, pop_lispvalue(second, 0));
	}

	delete_lispvalue(second);
	return first;
}

//----------------------------------------------------------------- Function ---
/**
 * Joins one or more Q-Expressions together
 *
 *   Ensure that all arguments are Q-Expression
 *   Join arguments
 *     -> pointer to joined Q-Expression
 */
static LispValue *builtin_join(LispEnv *env, LispValue *arguments)
{
	UNUSED(env);

	for (size_t i = 0; i < arguments->count; i++) {
		LVAL_ASSERT_TYPE("join", arguments, i, LVAL_QEXPR);
	}

	LispValue *joined = pop_lispvalue(arguments, 0);

	while (arguments->count) {
		joined = join_lispvalue(joined, pop_lispvalue(arguments, 0));
	}

	delete_lispvalue(arguments);
	return joined;
}
//----------------------------------------------------------------- Function ---
/**
 * Appends given LispValue first argument to the front of given following
 * Q-Expression
 *
 *   Ensure that there are 2 arguments
 *   Ensure that second argument is a Q-Expression
 *   Add an empty element to Q-Expression
 *   Shift all originals elements of Q-Expression forward over empty element
 *   Make first element of Q-Expression point to first argument
 *   Delete original arguments
 *     -> pointer to modified Q-Expression
 */
static LispValue *builtin_cons(LispEnv *env, LispValue *arguments)
{
	UNUSED(env);

	LVAL_ASSERT_COUNT("cons", arguments, 2);
	LVAL_ASSERT_TYPE("cons", arguments, 1, LVAL_QEXPR);

	LispValue *first_argument = pop_lispvalue(arguments, 0);
	LispValue *qexpr          = pop_lispvalue(arguments, 0);

	qexpr->count++;
	qexpr->cells = realloc(qexpr->cells, sizeof(LispValue *) * qexpr->count);

	memmove(&qexpr->cells[1],
	        &qexpr->cells[0],
	        sizeof(LispValue *) * (qexpr->count - 1));

	qexpr->cells[0] = first_argument;

	delete_lispvalue(arguments);
	return qexpr;
}

//----------------------------------------------------------------- Function ---
/**
 * Returns the number of elements in given Q-Expression
 *
 *   Ensure only a single argument is passed
 *   Ensure that argument is a Q-Expression
 *   Get the length of that Q-Expression
 *   Delete that Q-Expression
 *     -> length as new LispValue of type number

 */
static LispValue *builtin_len(LispEnv *env, LispValue *arguments)
{
	UNUSED(env);

	LVAL_ASSERT_COUNT("len", arguments, 1);
	LVAL_ASSERT_TYPE("len", arguments, 0, LVAL_QEXPR);

	LispValue *length =
	    new_lispvalue_number((double)arguments->cells[0]->count);

	delete_lispvalue(arguments);
	return length;
}

//----------------------------------------------------------------- Function ---
/**
 * Adds symbol, value pairs from a given Q-Expression list of symbols
 * and given list of LispValue to given LispEnv
 *
 *   Ensure first argument passed is a Q-Expression
 *   Ensure all elements listed in first argument are symbols
 *   Ensure passed list of LispValue count match symbols list count
 *   Add each symbol, value pair to given LispEnv
 *     -> pointer to LispValue error or empty S-Expression
 */
static LispValue *builtin_def(LispEnv *env, LispValue *arguments)
{
	LVAL_ASSERT_TYPE("def", arguments, 0, LVAL_QEXPR);

	LispValue *symbols = arguments->cells[0];

	for (size_t i = 0; i < symbols->count; i++) {
		LVAL_ASSERT(arguments,
		            (symbols->cells[i]->type == LVAL_SYMBOL),
		            "Function 'def' passed incorrect type for element %d !\n"
		            "\t expected %s, got %s.",
		            i,
		            lispvalue_type_tostring(LVAL_SYMBOL),
		            lispvalue_type_tostring(symbols->cells[i]->type));
	}

	LVAL_ASSERT(
	    arguments,
	    (symbols->count == arguments->count - 1),
	    "Function 'def' passed non-matching number of values and symbols !\n"
	    "\t got %d values and %d symbols.",
	    arguments->count - 1,
	    symbols->count);

	for (size_t i = 0; i < symbols->count; i++) {
		put_lispenv(env, symbols->cells[i], arguments->cells[i + 1]);
	}

	delete_lispvalue(arguments);
	return new_lispvalue_sexpr();
}

//----------------------------------------------------------------- Function ---
/**
 * Performs operation for given operator and LispValue representing all
 * the arguments to operate on
 *   -> pointer to Evaluation result LispValue
 */
static LispValue *
builtin_operator(LispEnv *env, LispValue *arguments, const char *operator)
{
	UNUSED(env);
	char *error_message = "";

	if (!(are_all_numbers(arguments))) {
		error_message = "Cannot operate on non-number !";
		goto exit_error;
	}

	LispValue *first_argument = pop_lispvalue(arguments, 0);

	/* unary negation */
	if (!(strcmp(operator, "-")) && arguments->count == 0) {
		first_argument->number = -(first_argument->number);
	}

	while (arguments->count > 0) {
		LispValue *next_argument = pop_lispvalue(arguments, 0);

		switch (*operator) {
		case '+':
			first_argument->number += next_argument->number;
			break;

		case '-':
			first_argument->number -= next_argument->number;
			break;

		case '*':
			first_argument->number *= next_argument->number;
			break;

		case '/':
			if (next_argument->number == 0) {
				delete_lispvalue(first_argument);
				// delete_lispvalue(next_argument);
				first_argument = new_lispvalue_error("Division by Zero !");
			}
			else {
				first_argument->number /= next_argument->number;
			}
			break;

		case '%':
			first_argument->number =
			    fmod(first_argument->number, next_argument->number);
			break;

		case '^':
			first_argument->number =
			    pow(first_argument->number, next_argument->number);
			break;

		case '>':
			first_argument->number =
			    (first_argument->number > next_argument->number)
			        ? first_argument->number
			        : next_argument->number;
			break;

		case '<':
			first_argument->number =
			    (first_argument->number < next_argument->number)
			        ? first_argument->number
			        : next_argument->number;
			break;
		default:
			break;
		}

		delete_lispvalue(next_argument);
	}

	// exit_success:
	delete_lispvalue(arguments);
	return first_argument;
exit_error:
	delete_lispvalue(arguments);
	return new_lispvalue_error(error_message);
}

//----------------------------------------------------------------- Function ---
/**
 * Registers given builtin function and  given name as symbol with given LispEnv
 *   -> Nothing
 */
static void add_builtin_lispenv(LispEnv *env, char *name, LispBuiltin function)
{
	LispValue *symbol = new_lispvalue_symbol(name);
	LispValue *value  = new_lispvalue_function(function);

	put_lispenv(env, symbol, value);
	delete_lispvalue(symbol);
	delete_lispvalue(value);
}

///----------------------------------------------------------------- Function ---
static void add_basicbuiltins_lispenv(LispEnv *env)
{
	add_builtin_lispenv(env, "add", builtin_add);
	add_builtin_lispenv(env, "sub", builtin_sub);
	add_builtin_lispenv(env, "mul", builtin_mul);
	add_builtin_lispenv(env, "div", builtin_div);
	add_builtin_lispenv(env, "mod", builtin_mod);
	add_builtin_lispenv(env, "pow", builtin_pow);
	add_builtin_lispenv(env, "max", builtin_max);
	add_builtin_lispenv(env, "min", builtin_min);
	add_builtin_lispenv(env, "+", builtin_add);
	add_builtin_lispenv(env, "-", builtin_sub);
	add_builtin_lispenv(env, "*", builtin_mul);
	add_builtin_lispenv(env, "/", builtin_div);
	add_builtin_lispenv(env, "%", builtin_mod);
	add_builtin_lispenv(env, "^", builtin_pow);
	add_builtin_lispenv(env, ">", builtin_max);
	add_builtin_lispenv(env, "<", builtin_min);

	add_builtin_lispenv(env, "head", builtin_head);
	add_builtin_lispenv(env, "tail", builtin_tail);
	add_builtin_lispenv(env, "list", builtin_list);
	add_builtin_lispenv(env, "init", builtin_init);
	add_builtin_lispenv(env, "eval", builtin_eval);
	add_builtin_lispenv(env, "join", builtin_join);
	add_builtin_lispenv(env, "cons", builtin_cons);
	add_builtin_lispenv(env, "len", builtin_len);
	add_builtin_lispenv(env, "def", builtin_def);

	add_builtin_lispenv(env, "env", builtin_env);
}

//----------------------------------------------------------------- Function ---
/**
 * Generates auto-completes matches from global word vocabulary
 *   -> Returns matches from vocabulary
 *
 * see https://thoughtbot.com/blog/tab-completion-in-gnu-readline
 */
// char *completion_generator(const char *text, const int state)
// {
// 	static int match_index, length;
// 	char *match;

// 	/*
// 	    * readline calls this function with state = 0 the first time
// 	    * this initialize once for the completion session
// 	    */
// 	if (!state) {
// 		match_index = 0;
// 		length      = strlen(text);
// 	}

// 	while ((match = vocabulary[match_index++])) {
// 		if (strncmp(match, text, length) == 0) {
// 			/* readline free() the returned string, correct ? */
// 			return strdup(match);
// 		}
// 	}

// 	return NULL;
// }

//----------------------------------------------------------------- Function ---
/**
 * Handles linenoise custom completion callbacks
 *   -> Nothing
 *
 * todo
 * - [ ] update to reflect current lispenv and not be a separate thing to
 *       maintain
 */
static void completion(const char *buf, linenoiseCompletions *lc)
{
	switch (buf[0]) {
	case 'a':
		linenoiseAddCompletion(lc, "add");
		break;
	case 'c':
		linenoiseAddCompletion(lc, "cons");
		break;
	case 'd':
		linenoiseAddCompletion(lc, "div");
		linenoiseAddCompletion(lc, "def");
		break;
	case 'e':
		linenoiseAddCompletion(lc, "env");
		linenoiseAddCompletion(lc, "eval");
		linenoiseAddCompletion(lc, "exit");
		break;
	case 'h':
		linenoiseAddCompletion(lc, "head");
		break;
	case 'i':
		linenoiseAddCompletion(lc, "init");
		break;
	case 'j':
		linenoiseAddCompletion(lc, "join");
		break;
	case 'l':
		linenoiseAddCompletion(lc, "list");
		linenoiseAddCompletion(lc, "len");
		break;
	case 'm':
		linenoiseAddCompletion(lc, "max");
		linenoiseAddCompletion(lc, "min");
		linenoiseAddCompletion(lc, "mod");
		linenoiseAddCompletion(lc, "mul");
		break;
	case 'p':
		linenoiseAddCompletion(lc, "pow");
		break;
	case 's':
		linenoiseAddCompletion(lc, "sub");
		break;
	case 't':
		linenoiseAddCompletion(lc, "tail");
		break;
	default:
		// for (size_t i = 0; i < ARRAY_LENGTH(vocabulary); i++) {
		// 	linenoiseAddCompletion(lc, vocabulary[i]);
		// }
		break;
	}
}
//----------------------------------------------------------------- Function ---
/**
 * Handles linenoise hints callback
 *   -> pointer to hint string
 *
 * todo
 * - [ ] update to reflect current lispenv and not be a separate thing to
 *       maintain
 */
static char *hints(const char *buf, int *color, int *bold)
{
	if (!strcasecmp(buf, "head")) {
		*color = 35;
		*bold  = 0;
		return " {qexpr}";
	}
	return NULL;
}

//--------------------------------------------------------------------- MAIN ---
/**
 * Setup mpc parsers
 * Setup linenoise
 * Print intro
 * Take and Process user input
 * Cleanup
 *   -> Error code
 *
 * todo
 * - [ ] have a go at const correctness
 * - [ ] use current LispEnv symbols directly for hints and auto-completion
 * - [ ] prevent def from rewriting default builtins
 */
// static char input[2048];

int main()
{
	//--------------------------------------------------------------- parser
	const mpc_parser_t *number_parser = mpc_new("number");
	const mpc_parser_t *symbol_parser = mpc_new("symbol");
	const mpc_parser_t *sexpr_parser  = mpc_new("sexpr");
	const mpc_parser_t *qexpr_parser  = mpc_new("qexpr");
	const mpc_parser_t *expr_parser   = mpc_new("expr");
	mpc_parser_t *lispy_parser        = mpc_new("lispy");

	mpca_lang(MPCA_LANG_DEFAULT,
	          "number   : /[-]?[0-9]+[.]?[0-9]*([eE][-+]?[0-9]+)?/ ;"
	          "symbol   : /[a-zA-Z0-9_+\\-*%^\\/\\\\=<>!&]+/ ;"
	          "sexpr    : '(' <expr>* ')' ;"
	          "qexpr    : '{' <expr>* '}' ;"
	          "expr     : <number> | <symbol> | <sexpr> | <qexpr> ;"
	          "lispy    : /^/ <expr>* /$/ ;",
	          number_parser,
	          symbol_parser,
	          sexpr_parser,
	          qexpr_parser,
	          expr_parser,
	          lispy_parser);

	//------------------------------------------------------------ linenoise
	// linenoiseSetMultiLine(1);
	/* Set the completion callback. This will be called every time the
	 * user uses the <tab> key. */
	linenoiseSetCompletionCallback(completion);
	linenoiseSetHintsCallback(hints);
	/* Load history at startup, hard set history length */
	linenoiseHistoryLoad("lispy_history.txt");
	linenoiseHistorySetMaxLen(16);

	//---------------------------------------------------------------- intro
	puts(FG_BRIGHT_BLUE REVERSE " Lispy version 0.11.6 " RESET FG_BRIGHT_BLUE
	                            " type exit to quit" RESET);

	//----------------------------------------------------- lisp environment
	LispEnv *lispenv = new_lispenv();
	add_basicbuiltins_lispenv(lispenv);

	//----------------------------------------------------------- input loop
	char *prompt = BG_BRIGHT_GREEN FG_GREEN "lispy " FG_BLACK "> " RESET;

	for (;;) {
		// print_prompt();
		char *input = linenoise(prompt);
		// fgets(input, 2048, stdin);

		if (input != NULL) {
			if ((strcmp(input, "exit")) == 0) {
				// if (strstr(input, "exit")) {
				XFREE(input, "main input loop");
				break;
			}

			linenoiseHistoryAdd(input);
			linenoiseHistorySave("lispy_history.txt");

			/* try to parse input */
			mpc_result_t mpc_result;

			if (mpc_parse("<stdin>", input, lispy_parser, &mpc_result)) {
#ifdef DEBUG_MPC
				mpc_ast_print(mpc_result.output);
#endif

				LispValue *lispvalue_result = read_lispvalue(mpc_result.output);
#ifdef DEBUG_MPC
				print_lispvalue_newline(lispenv, lispvalue_result);
#endif

				lispvalue_result = eval_lispvalue(lispenv, lispvalue_result);
				print_lispvalue_newline(lispenv, lispvalue_result);
				delete_lispvalue(lispvalue_result);

				mpc_ast_delete(mpc_result.output);
			}
			else {
				mpc_err_print(mpc_result.error);
				mpc_err_delete(mpc_result.error);
			}

			/* readline malloc'd input*/
			XFREE(input, "main input loop");
		}
	}

	//-------------------------------------------------------------- cleanup
	delete_lispenv(lispenv);
	mpc_cleanup(6,
	            number_parser,
	            symbol_parser,
	            sexpr_parser,
	            qexpr_parser,
	            expr_parser,
	            lispy_parser);

	return 0;
}
