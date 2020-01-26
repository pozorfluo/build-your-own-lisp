#include <stdio.h>
#include <stdlib.h>

#include <editline/history.h>
#include <editline/readline.h>
#include <string.h>

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

//------------------------------------------------------------------- MACROS ---
#define LVAL_ASSERT_CONDITION(args, condition, symbol, message)                \
	if (!(condition)) {                                                        \
		delete_lispvalue(args);                                                \
		return new_lispvalue_error("Function " symbol message);                 \
	}

#define LVAL_ASSERT_ARG(args, expected_count, symbol)                          \
	if (args->count != expected_count) {                                       \
		delete_lispvalue(args);                                                \
		return new_lispvalue_error("Function " symbol                          \
		                           " passed incorrect number of arguments !"); \
	}

#define LVAL_ASSERT_NONEMPTY(args, symbol)                                     \
	if (args->cells[0]->count == 0) {                                          \
		delete_lispvalue(args);                                                \
		return new_lispvalue_error("Function " symbol " passed {} !");         \
	}

#define LVAL_ASSERT_TYPE(args, arg_index, expected_type, symbol)               \
	if (args->cells[arg_index]->type != expected_type) {                       \
		delete_lispvalue(args);                                                \
		return new_lispvalue_error("Function " symbol                          \
		                           " passed incorrect types !");               \
	}

#define LVAL_DUMP(args)                                                        \
	printf(#args                                                               \
	       "\n"                                                                \
	       "\ttype   : %d\n"                                                   \
	       "\tnumber : %f\n"                                                   \
	       "\terror  : %s\n"                                                   \
	       "\tsymbol : %s\n"                                                   \
	       "\tcount  : %d\n\n",                                                \
	       args->type,                                                         \
	       args->number,                                                       \
	       args->error,                                                        \
	       args->symbol,                                                       \
	       args->count);

/**
 * Suppresses compiler warning about unused parameters needed in
 * function signatures
 *
 * Debating whether to use gcc specific __attribute__((unused)) or this
 */
#define UNUSED(_parameter) (void)(_parameter)

//------------------------------------------------------------- DEBUG MACROS ---

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

	double number;
	char *error;
	char *symbol;
	LispBuiltin function;

	int count;
	struct LispValue **cells;
};

struct LispEnv {
	/* entries in each list match their corresponding */
	/* entry in the other list by index */
	int count;
	char **symbols;
	LispValue **values;
};

//--------------------------------------------------------------- PROTOTYPES ---
/*
 * todo
 * - [ ] rename parameters to be more consistent overall but still locally
 * unambiguously relevant
 */
LispValue *
builtin_operator(LispEnv *lispenv, LispValue *arguments, const char *operator);
LispValue *lookup_builtin(LispValue *arguments, const char *symbol);
LispValue *take_lispvalue(LispValue *lispvalue, int index);
LispValue *pop_lispvalue(LispValue *lispvalue, int index);
LispValue *eval_lispvalue(LispEnv *lispenv, LispValue *lispvalue);
LispValue *copy_lispvalue(LispValue *lispvalue);
void print_lispvalue(LispValue *lispvalue);

//-------------------------------------------------- STATIC GLOBAL VARIABLES ---
/* Readline auto-completion configuration */
static char *vocabulary[] = {"list",
                             "head",
                             "tail",
                             "join",
                             "cons",
                             "len",
                             "init",
                             "eval",
                             "+",
                             "-",
                             "*",
                             "/",
                             "%",
                             "^",
                             //  "min",
                             //  "max",
                             //  "~",
                             //  "!",
                             //  "!=",
                             //  "√",
                             //  "Σ",
                             //  "≥",
                             //  "≤",
                             ">", // used as min
                             "<", // used as max
                                  //  "=",
                                  //  ">=",
                                  //  "<=",
                                  //  "...",
                             "{",
                             "}",
                             "(",
                             ")",
                             NULL};

//----------------------------------------------------------------- Function ---
/**
 * Creates a new LispValue of type number for a given a number
 *   -> pointer to new LispValue number
 */
LispValue *new_lispvalue_number(const double number)
{
	LispValue *new_value = malloc(sizeof(LispValue));

	new_value->type   = LVAL_NUMBER;
	new_value->number = number;

	return new_value;
}

//----------------------------------------------------------------- Function ---
/**
 * Creates a new LispValue of type error for a given a error message
 *   -> pointer to new LispValue error
 */
LispValue *new_lispvalue_error(const char *message)
{
	LispValue *new_value = malloc(sizeof(LispValue));

	new_value->type  = LVAL_ERR;
	new_value->error = malloc(strlen(message) + 1);
	strcpy(new_value->error, message);

	return new_value;
}

//----------------------------------------------------------------- Function ---
/**
 * Creates a new LispValue of type symbol for a given a symbol
 *   -> pointer to new LispValue symbol
 */
LispValue *new_lispvalue_symbol(const char *symbol)
{
	LispValue *new_value = malloc(sizeof(LispValue));

	new_value->type   = LVAL_SYMBOL;
	new_value->symbol = malloc(strlen(symbol) + 1);
	strcpy(new_value->symbol, symbol);

	return new_value;
}

//----------------------------------------------------------------- Function ---
/**
 * Creates a new empty LispValue of type function
 *   -> pointer to new LispValue function
 */
LispValue *new_lispvalue_function(LispBuiltin function)
{
	LispValue *new_value = malloc(sizeof(LispValue));

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
LispEnv *new_lispenv(void)
{
	LispEnv *new_env = malloc(sizeof(LispEnv));
	new_env->count   = 0;
	new_env->symbols = NULL;
	new_env->values  = NULL;
	return new_env;
}

//----------------------------------------------------------------- Function ---
/**
 * Creates a new empty LispValue of type sexpr
 *   -> pointer to new LispValue sexpr
 *
 * todo
 * - [ ] init unused field to dummy value to avoid undefined behaviour
 */

LispValue *new_lispvalue_sexpr(void)
{
	LispValue *new_value = malloc(sizeof(LispValue));

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
LispValue *new_lispvalue_qexpr(void)
{
	LispValue *new_value = malloc(sizeof(LispValue));

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
void delete_lispvalue(LispValue *lispvalue)
{
	switch (lispvalue->type) {
	case LVAL_NUMBER:
		break;

	case LVAL_ERR:

		free(lispvalue->error);
		break;

	case LVAL_SYMBOL:
		free(lispvalue->symbol);
		break;

	case LVAL_FUNCTION:
		break;

	case LVAL_SEXPR:
	case LVAL_QEXPR:
		for (int i = 0; i < lispvalue->count; i++) {
			delete_lispvalue(lispvalue->cells[i]);
		}

		free(lispvalue->cells);
		break;

	default:
		break;
	}

	free(lispvalue);
}

//----------------------------------------------------------------- Function ---
/**
 * Frees given LispEnv ressources
 *   -> nothing
 */
void delete_lispenv(LispEnv *env)
{
	for (int i = 0; i < env->count; i++) {
		free(env->symbols[i]);
		delete_lispvalue(env->values[i]);
	}
	free(env->symbols);
	free(env->values);
	free(env);
}

//----------------------------------------------------------------- Function ---
/**
 * Gets LispValue associated with given LispValue symbol if any
 *   -> pointer to new copy of associated LispValue
 *
 * Iterate over all items in environment
 *   If stored symbol match given Lispvalue symbol
 *     -> a copy of associated value in environment
 *   -> an error if no symbol match found
 */
LispValue *get_lispenv(LispEnv *env, LispValue *value)
{
	for (int i = 0; i < env->count; i++) {
		if (strcmp(env->symbols[i], value->symbol) == 0) {
			return copy_lispvalue(env->values[i]);
		}
	}
	return new_lispvalue_error("unbound symbol !");
}

//----------------------------------------------------------------- Function ---
/**
 * Creates a new entry in given LispEnvironment for given symbol, value pair
 * or replaces value associated with existing symbol in given LispEnvironment
 * with given value
 *   -> nothing
 *
 * Iterate over all items in environment
 *   If symbol is found
 *     Delete value at that position
 *     Replace with copy of given value
 * 	     -> nothing
 * Update entry count
 * Update lists size
 * Copy contents of given value and symbol into new entry
 *   -> nothing
 */
void put_lispenv(LispEnv *env, LispValue *symbol, LispValue *value)
{
	for (int i = 0; i < env->count; i++) {
		if (strcmp(env->symbols[i], symbol->symbol) == 0) {
			delete_lispvalue(env->values[i]);
			env->values[i] = copy_lispvalue(value);
			return;
		}
	}

	env->count++;
	env->symbols = realloc(env->symbols, sizeof(char *) * env->count);
	env->values  = realloc(env->values, sizeof(LispValue *) * env->count);

	env->symbols[env->count - 1] = malloc(strlen(symbol->symbol) + 1);
	strcpy(env->symbols[env->count - 1], symbol->symbol);
	env->values[env->count - 1] = copy_lispvalue(value);
}

//----------------------------------------------------------------- Function ---
/**
 * Evaluates given AST node of type number
 *   -> pointer to a LispValue number
 */
LispValue *read_lispvalue_number(mpc_ast_t *ast)
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
LispValue *add_lispvalue(LispValue *expr, LispValue *lispvalue)
{
	expr->count++;
	expr->cells = realloc(expr->cells, sizeof(LispValue *) * expr->count);
	expr->cells[expr->count - 1] = lispvalue;

	return expr;
}

//----------------------------------------------------------------- Function ---
/**
 * Traverses given AST and evaluates the expression
 *   -> pointer to a LispValue
 */
LispValue *read_lispvalue(mpc_ast_t *ast)
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
 *
 *   -> pointer to new LispValue
 */
LispValue *copy_lispvalue(LispValue *lispvalue)
{
	LispValue *copy = malloc(sizeof(LispValue));
	copy->type      = lispvalue->type;

	switch (lispvalue->type) {
	case LVAL_NUMBER:
		copy->number = lispvalue->number;
		break;

	case LVAL_FUNCTION:
		copy->function = lispvalue->function;
		break;

	case LVAL_ERR:
		copy->error = malloc(strlen(lispvalue->error) + 1);
		strcpy(copy->error, lispvalue->error);
		break;

	case LVAL_SYMBOL:
		copy->symbol = malloc(strlen(lispvalue->symbol) + 1);
		strcpy(copy->symbol, lispvalue->symbol);
		break;

	case LVAL_SEXPR:
	case LVAL_QEXPR:
		copy->count = lispvalue->count;
		copy->cells = malloc(sizeof(LispValue *) * copy->count);
		for (int i = 0; i < copy->count; i++) {
			copy->cells[i] = copy_lispvalue(lispvalue->cells[i]);
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
void print_lispvalue_expr(LispValue *lispvalue,
                          const char open,
                          const char close)
{
	putchar(open);

	for (int i = 0; i < lispvalue->count; i++) {
		print_lispvalue(lispvalue->cells[i]);
		putchar(' ');
	}

	putchar(close);
}

//----------------------------------------------------------------- Function ---
/**
 * Pretty prints given LispValue according to its type
 *   -> Nothing
 */
void print_lispvalue(LispValue *lispvalue)
{
	switch (lispvalue->type) {
	case LVAL_NUMBER:
		printf(FG_YELLOW "%f" RESET, lispvalue->number);
		break;

	case LVAL_ERR:
		printf(FG_RED "Error : %s" RESET, lispvalue->error);
		break;

	case LVAL_SYMBOL:
		printf(FG_CYAN "%s" RESET, lispvalue->symbol);
		break;

	case LVAL_FUNCTION:
		printf(FG_GREEN "<function>" RESET);
		break;

	case LVAL_SEXPR:
		print_lispvalue_expr(lispvalue, '(', ')');
		break;

	case LVAL_QEXPR:
		print_lispvalue_expr(lispvalue, '{', '}');
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
void print_lispvalue_newline(LispValue *lispvalue)
{
	print_lispvalue(lispvalue);
	putchar('\n');
}

//----------------------------------------------------------------- Function ---
/**
 * Evaluates given LispValue of type sexpr
 *   -> pointer to result LispValue
 */
LispValue *eval_lispvalue_sexpr(LispEnv *lispenv, LispValue *lispvalue)
{
	/* evaluate children */
	for (int i = 0; i < lispvalue->count; i++) {
		lispvalue->cells[i] = eval_lispvalue(lispenv, lispvalue->cells[i]);
	}

	/* check for errors */
	for (int i = 0; i < lispvalue->count; i++) {
		if (lispvalue->cells[i]->type == LVAL_ERR) {
			return take_lispvalue(lispvalue, i);
		}
	}

	/* empty expression */
	if (lispvalue->count == 0) {
		return lispvalue;
	}
	/* single expression */
	else if (lispvalue->count == 1) {
		return take_lispvalue(lispvalue, 0);
	}
	else {
		/* make sure first element is a function */
		LispValue *first_element = pop_lispvalue(lispvalue, 0);
		if (first_element->type != LVAL_FUNCTION) {
			delete_lispvalue(first_element);
			delete_lispvalue(lispvalue);
			return new_lispvalue_error("first element is not a function !");
		}
		else {
			LispValue *result = first_element->function(lispenv, lispvalue);
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
LispValue *eval_lispvalue(LispEnv *lispenv, LispValue *lispvalue)
{
	if (lispvalue->type == LVAL_SYMBOL) {
		LispValue *retrieved_lispvalue = get_lispenv(lispenv, lispvalue);
		delete_lispvalue(lispvalue);
		return retrieved_lispvalue;
	}
	else if (lispvalue->type == LVAL_SEXPR) {
		return eval_lispvalue_sexpr(lispenv, lispvalue);
	}
	else {
		return lispvalue;
	}
}

//----------------------------------------------------------------- Function ---
/**
 * Extracts single element from given LispValue of type sexpr
 * Shifts the rest of element list pointer backward over extracted element
 * pointer
 *   -> Extracted LispValue
 */
LispValue *pop_lispvalue(LispValue *lispvalue, const int index)
{
	LispValue *extracted_element = lispvalue->cells[index];

	memmove(&lispvalue->cells[index],
	        &lispvalue->cells[index + 1],
	        sizeof(LispValue *) * (lispvalue->count - index - 1));

	lispvalue->count--;

	lispvalue->cells =
	    realloc(lispvalue->cells, sizeof(LispValue *) * lispvalue->count);

	return extracted_element;
}

//----------------------------------------------------------------- Function ---
/**
 * Extracts single element from given LispValue of type sexpr
 * Deletes the rest
 *   -> Extracted LispValue
 */
LispValue *take_lispvalue(LispValue *lispvalue, const int index)
{
	LispValue *extracted_element = pop_lispvalue(lispvalue, index);
	delete_lispvalue(lispvalue);

	return extracted_element;
}

//----------------------------------------------------------------- Function ---
/**
 * Goes over all elements of given LispValue
 * Checks that they are all of type number
 *   -> Truth value of predicate
 *
 */
int are_all_numbers(LispValue *arguments)
{
	// int result = 1;
	for (int i = 0; i < arguments->count; i++) {
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
LispValue *builtin_add(LispEnv *env, LispValue *value)
{
	return builtin_operator(env, value, "+");
}

LispValue *builtin_sub(LispEnv *env, LispValue *value)
{
	return builtin_operator(env, value, "-");
}

LispValue *builtin_mul(LispEnv *env, LispValue *value)
{
	return builtin_operator(env, value, "*");
}

LispValue *builtin_div(LispEnv *env, LispValue *value)
{
	return builtin_operator(env, value, "/");
}

LispValue *builtin_mod(LispEnv *env, LispValue *value)
{
	return builtin_operator(env, value, "%");
}

LispValue *builtin_pow(LispEnv *env, LispValue *value)
{
	return builtin_operator(env, value, "^");
}

LispValue *builtin_max(LispEnv *env, LispValue *value)
{
	return builtin_operator(env, value, ">");
}

LispValue *builtin_min(LispEnv *env, LispValue *value)
{
	return builtin_operator(env, value, "<");
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
LispValue *builtin_head(LispEnv *env, LispValue *arguments)
{
	UNUSED(env);

	LVAL_ASSERT_ARG(arguments, 1, "'head'");
	LVAL_ASSERT_TYPE(arguments, 0, LVAL_QEXPR, "'head'");
	LVAL_ASSERT_NONEMPTY(arguments, "'head'");

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
LispValue *builtin_tail(LispEnv *env, LispValue *arguments)
{
	UNUSED(env);

	LVAL_ASSERT_ARG(arguments, 1, "'tail'");
	LVAL_ASSERT_TYPE(arguments, 0, LVAL_QEXPR, "'tail'");
	LVAL_ASSERT_NONEMPTY(arguments, "'tail'");

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
LispValue *builtin_list(LispEnv *env, LispValue *arguments)
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
LispValue *builtin_init(LispEnv *env, LispValue *arguments)
{
	UNUSED(env);

	LVAL_ASSERT_ARG(arguments, 1, "'init'");
	LVAL_ASSERT_TYPE(arguments, 0, LVAL_QEXPR, "'init'");
	LVAL_ASSERT_NONEMPTY(arguments, "'init'");

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
LispValue *builtin_eval(LispEnv *env, LispValue *arguments)
{
	UNUSED(env);

	LVAL_ASSERT_ARG(arguments, 1, "'eval'");
	LVAL_ASSERT_TYPE(arguments, 0, LVAL_QEXPR, "'eval'");

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
LispValue *join_lispvalue(LispValue *first, LispValue *second)
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
LispValue *builtin_join(LispEnv *env, LispValue *arguments)
{
	UNUSED(env);

	for (int i = 0; i < arguments->count; i++) {
		LVAL_ASSERT_TYPE(arguments, i, LVAL_QEXPR, "'join'");
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
LispValue *builtin_cons(LispEnv *env, LispValue *arguments)
{
	UNUSED(env);

	LVAL_ASSERT_ARG(arguments, 2, "'cons'");
	LVAL_ASSERT_TYPE(arguments, 1, LVAL_QEXPR, "'cons'");

	LispValue *first_argument = pop_lispvalue(arguments, 0);
	LispValue *qexpr          = pop_lispvalue(arguments, 0);

	qexpr->count++;
	qexpr->cells = realloc(qexpr->cells, sizeof(LispValue *) * qexpr->count);

	memmove(&qexpr->cells[1],
	        &qexpr->cells[0],
	        sizeof(LispValue *) * (qexpr->count - 1));

	qexpr->cells[0] = first_argument;

	// todo
	//   [] find out why this is necessary after asserting 2 arg and poping
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
LispValue *builtin_len(LispEnv *env, LispValue *arguments)
{
	UNUSED(env);

	LVAL_ASSERT_ARG(arguments, 1, "'len'");
	LVAL_ASSERT_TYPE(arguments, 0, LVAL_QEXPR, "'len'");

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
LispValue *builtin_def(LispEnv *env, LispValue *arguments)
{
	LVAL_ASSERT_TYPE(arguments, 0, LVAL_QEXPR, "'def'");

	LispValue *symbols = arguments->cells[0];

	for (int i = 0; i < symbols->count; i++) {
		// if we use this macro we never delete_lispvalue(arguments)
		// LVAL_ASSERT_TYPE(symbols, i, LVAL_SYMBOL, "'def'");
		LVAL_ASSERT_CONDITION(arguments,
		                      symbols->cells[i]->type == LVAL_SYMBOL,
		                      "'def'",
		                      " cannot define non-symbol !");
	}

	LVAL_ASSERT_CONDITION(arguments,
	                      symbols->count == arguments->count - 1,
	                      "'def'",
	                      " passed non-matching number of values and symbols");

	for (int i = 0; i < symbols->count; i++) {
		put_lispenv(env, symbols->cells[i], arguments->cells[i + 1]);
	}

	delete_lispvalue(arguments);
	return new_lispvalue_sexpr();
}

//----------------------------------------------------------------- Function ---
/**
 * Performs operation for given operator and LispValue representing all
 * the arguments to operate on
 *   -> Evaluation result LispValue
 */
LispValue *
builtin_operator(LispEnv *env, LispValue *arguments, const char *operator)
{
	UNUSED(env);

	if (!(are_all_numbers(arguments))) {
		delete_lispvalue(arguments);
		return new_lispvalue_error("Cannot operate on non-number !");
	}
	else {
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

		delete_lispvalue(arguments);
		return first_argument;
	}
}

//----------------------------------------------------------------- Function ---
/**
 * Registers given builtin function and  given name as symbol with given LispEnv
 *   -> Nothing
 */
void add_builtin_lispenv(LispEnv *env, char *name, LispBuiltin function)
{
	LispValue *symbol = new_lispvalue_symbol(name);
	LispValue *value  = new_lispvalue_function(function);

	put_lispenv(env, symbol, value);
	delete_lispvalue(symbol);
	delete_lispvalue(value);
}

void add_basicbuiltins_lispenv(LispEnv *env)
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
}

//----------------------------------------------------------------- Function ---
/**
 * Generates auto-completes matches from global word vocabulary
 *   -> Returns matches from vocabulary
 *
 * see https://thoughtbot.com/blog/tab-completion-in-gnu-readline
 */
char *completion_generator(const char *text, const int state)
{
	static int match_index, length;
	char *match;

	/*
	    * readline calls this function with state = 0 the first time
	    * this initialize once for the completion session
	    */
	if (!state) {
		match_index = 0;
		length      = strlen(text);
	}

	while ((match = vocabulary[match_index++])) {
		if (strncmp(match, text, length) == 0) {
			/* readline free() the returned string, correct ? */
			return strdup(match);
		}
	}

	return NULL;
}

//----------------------------------------------------------------- Function ---
/**
 * Handles custom completion registered to readline global variable
 *   -> Completion matches
 *
 * todo
 * - [ ] fix workaround compiler warning for expected **completer prototype
 */
char **completer(const char *text, const int start, const int end)
{
	/* not doing filename completion even if 0 matches */
	// rl_attempted_completion_over = 1;

	/* readline expects char** fn(char*, int, int) */
	/* temp workaround compiler warnings for unused-parameters */
	UNUSED(start);
	UNUSED(end);

	return rl_completion_matches(text, &completion_generator);
}

//----------------------------------------------------------------- Function ---
/**
 * pretty prints a prompt
 *   -> Nothing
 */
void print_prompt() { fputs(BOLD FG_GREEN "lispy> " RESET, stdout); }

//--------------------------------------------------------------------- MAIN ---
/**
 * Setup parsers
 * Setup completer
 * Print intro
 * Take and Process user input
 * Cleanup parsers
 *   -> Error code
 *
 * todo
 * - [ ] have a go at const correctness
 */
int main()
{
	//----------------------------------------------------------- playground

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

	//------------------------------------------------------------ completer
	/* register custom completer with readline global variable */
	rl_attempted_completion_function = &completer;

	//---------------------------------------------------------------- intro
	puts(BG_BLUE "Lispy version 0.0.0.0.1 " RESET FG_BRIGHT_CYAN
	             "to Exit press CTRL + C" RESET);

	//----------------------------------------------------- lisp environment
	LispEnv *lispenv = new_lispenv();
	add_basicbuiltins_lispenv(lispenv);

	//----------------------------------------------------------- input loop
	for (;;) {
		print_prompt();

		/* readline doesn't like escape codes :/ */
		char *input = readline("");

		if (input != NULL) {
			add_history(input);

			/* try to parse input */
			mpc_result_t mpc_result;

			if (mpc_parse("<stdin>", input, lispy_parser, &mpc_result)) {
				mpc_ast_print(mpc_result.output);

				LispValue *lispvalue_result = read_lispvalue(mpc_result.output);
				print_lispvalue_newline(lispvalue_result);

				lispvalue_result = eval_lispvalue(lispenv, lispvalue_result);
				print_lispvalue_newline(lispvalue_result);
				delete_lispvalue(lispvalue_result);

				mpc_ast_delete(mpc_result.output);
			}
			else {
				mpc_err_print(mpc_result.error);
				mpc_err_delete(mpc_result.error);
			}

			/* readline malloc'd input*/
			free(input);
		}
	}

	//-------------------------------------------------------------- cleanup
	mpc_cleanup(6,
	            number_parser,
	            symbol_parser,
	            sexpr_parser,
	            qexpr_parser,
	            expr_parser,
	            lispy_parser);

	return 0;
}
