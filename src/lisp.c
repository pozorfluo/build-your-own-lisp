#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <editline/readline.h>
#include <editline/history.h>

#include "../include/mpc.h"

//---------------------------------------------------------- PLATFORM MACROS ---


//-------------------------------------------------------------- ANSI MACROS ---
/*
 * see https://invisible-island.net/xterm/ctlseqs/ctlseqs.html
 *
 * todo
 *   [] clarify which are ready-made sequences
 *   [] clarify which are bits to be expanded into sequence
 *
 */
#define ESC "\x1b"

#define RESET      ESC"[0m"
#define BOLD       ESC"[1m"
#define UNDERSCORE ESC"[4m"
#define BLINK      ESC"[5m"
#define REVERSE    ESC"[7m"
#define CONCEALED  ESC"[8m"

#define FG "3"
#define BG "4"
#define FG_BRIGHT "9"
#define BG_BRIGHT "10"

#define BLACK    "0"
#define RED      "1"
#define GREEN    "2"
#define YELLOW   "3"
#define BLUE     "4"
#define MAGENTA  "5"
#define CYAN     "6"
#define WHITE    "7"
#define DEFAULT  "9"

#define FG_BLACK     ESC"["FG BLACK  "m"
#define FG_RED       ESC"["FG RED    "m"
#define FG_GREEN     ESC"["FG GREEN  "m"
#define FG_YELLOW    ESC"["FG YELLOW "m"
#define FG_BLUE      ESC"["FG BLUE   "m"
#define FG_MAGENTA   ESC"["FG MAGENTA"m"
#define FG_CYAN      ESC"["FG CYAN   "m"
#define FG_WHITE     ESC"["FG WHITE  "m"
#define FG_DEFAULT   ESC"["FG DEFAULT"m"

#define BG_BLACK     ESC"["BG BLACK  "m"
#define BG_RED       ESC"["BG RED    "m"
#define BG_GREEN     ESC"["BG GREEN  "m"
#define BG_YELLOW    ESC"["BG YELLOW "m"
#define BG_BLUE      ESC"["BG BLUE   "m"
#define BG_MAGENTA   ESC"["BG MAGENTA"m"
#define BG_CYAN      ESC"["BG CYAN   "m"
#define BG_WHITE     ESC"["BG WHITE  "m"
#define BG_DEFAULT   ESC"["BG DEFAULT"m"

#define FG_BRIGHT_BLACK     ESC"["FG_BRIGHT BLACK  "m"
#define FG_BRIGHT_RED       ESC"["FG_BRIGHT RED    "m"
#define FG_BRIGHT_GREEN     ESC"["FG_BRIGHT GREEN  "m"
#define FG_BRIGHT_YELLOW    ESC"["FG_BRIGHT YELLOW "m"
#define FG_BRIGHT_BLUE      ESC"["FG_BRIGHT BLUE   "m"
#define FG_BRIGHT_MAGENTA   ESC"["FG_BRIGHT MAGENTA"m"
#define FG_BRIGHT_CYAN      ESC"["FG_BRIGHT CYAN   "m"
#define FG_BRIGHT_WHITE     ESC"["FG_BRIGHT WHITE  "m"
#define FG_BRIGHT_DEFAULT   ESC"["FG_BRIGHT DEFAULT"m"

#define BG_BRIGHT_BLACK     ESC"["BG_BRIGHT BLACK  "m"
#define BG_BRIGHT_RED       ESC"["BG_BRIGHT RED    "m"
#define BG_BRIGHT_GREEN     ESC"["BG_BRIGHT GREEN  "m"
#define BG_BRIGHT_YELLOW    ESC"["BG_BRIGHT YELLOW "m"
#define BG_BRIGHT_BLUE      ESC"["BG_BRIGHT BLUE   "m"
#define BG_BRIGHT_MAGENTA   ESC"["BG_BRIGHT MAGENTA"m"
#define BG_BRIGHT_CYAN      ESC"["BG_BRIGHT CYAN   "m"
#define BG_BRIGHT_WHITE     ESC"["BG_BRIGHT WHITE  "m"
#define BG_BRIGHT_DEFAULT   ESC"["BG_BRIGHT DEFAULT"m"

#define DEFAULT_COLORS  ESC"[39;49m"

/* codes in the sequence must be separated by ";" */
#define ANSI(SEQUENCE) ESC"["SEQUENCE"m"

//------------------------------------------------------------- DEBUG MACROS ---

//-------------------------------------------------------------------- ENUMS ---
enum LispValueType
{
    LVAL_ERR,
    LVAL_NUMBER,
    LVAL_SYMBOL,
    LVAL_SEXPR
};

//----------------------------------------------------------------- TYPEDEFS ---
typedef struct LispValue
{
    int type;
    double number;
    char *error;
    char *symbol;
    /* sexpr */
    int count;
    struct LispValue **cells;
} LispValue;

//--------------------------------------------------------------- PROTOTYPES ---
LispValue* new_lispvalue_number(double number);
LispValue* new_lispvalue_error(char *message);
LispValue* new_lispvalue_symbol(char *symbol);
LispValue* new_lispvalue_sexpr(void);
void delete_lispvalue(LispValue *lispvalue);
LispValue* read_lispvalue_number(mpc_ast_t *ast);
LispValue* add_lispvalue(LispValue *sexpr, LispValue *lispvalue);
LispValue* read_lispvalue(mpc_ast_t *ast);
void print_lispvalue_sexpr(LispValue *lispvalue, char open, char close);
void print_lispvalue(LispValue *lispvalue);
void print_lispvalue_newline(LispValue *lispvalue);
LispValue* eval_lispvalue_sexpr(LispValue *lispvalue);
LispValue* eval_lispvalue(LispValue *lispvalue);
LispValue* pop_lispvalue(LispValue *lispvalue, int index);
LispValue* take_lispvalue(LispValue *lispvalue, int index);
int are_all_numbers(LispValue *arguments);
LispValue* eval_lispvalue_operator(LispValue *arguments, char *operator);
char* completion_generator(const char *text, int state);
char** completer(const char *text, int start, int end);
void print_prompt();
//-------------------------------------------------- STATIC GLOBAL VARIABLES ---
/* Readline auto-completion configuration */
static char *vocabulary[] =
{
    "fourcheau",
    "monparounaze",
    "karl",
    "lagerfeld",
    "+",
    "-",
    "*",
    "/",
    "%",
    "^",
    ">",
    "<",
    NULL
};


//----------------------------------------------------------------- FUNCTION ---
/**
 * creates a new LispValue of type number for a given a number
 *
 *   -> pointer to a LispValue number
 *
 * time  O(?)
 * space O(?)
 */
LispValue* new_lispvalue_number(double number)
{
    LispValue *new_value = malloc(sizeof(LispValue));

    new_value->type = LVAL_NUMBER;
    new_value->number = number;

    return new_value;
}

//----------------------------------------------------------------- FUNCTION ---
/**
 * creates a new LispValue of type error for a given a error message
 *
 *   -> pointer to a LispValue error
 *
 * time  O(?)
 * space O(?)
 */
LispValue* new_lispvalue_error(char *message)
{
    LispValue *new_value = malloc(sizeof(LispValue));

    new_value->type = LVAL_ERR;
    new_value->error = malloc(strlen(message) + 1);
    strcpy(new_value->error, message);

    return new_value;
}

//----------------------------------------------------------------- FUNCTION ---
/**
 * creates a new LispValue of type symbol for a given a symbol
 *
 *   -> pointer to a LispValue symbol
 *
 * time  O(?)
 * space O(?)
 */
LispValue* new_lispvalue_symbol(char *symbol)
{
    LispValue *new_value = malloc(sizeof(LispValue));

    new_value->type = LVAL_SYMBOL;
    // printf(FG_BRIGHT_RED "\n%s\n" RESET, symbol);
    new_value->symbol = malloc(strlen(symbol) + 1);

    strcpy(new_value->symbol, symbol);

    return new_value;
}

//----------------------------------------------------------------- FUNCTION ---
/**
 * Creates a new empty LispValue of type sexpr
 *
 *   -> pointer to a LispValue sexpr
 *
 * time  O(?)
 * space O(?)
 */
LispValue* new_lispvalue_sexpr(void)
{
    LispValue *new_value = malloc(sizeof(LispValue));

    new_value->type = LVAL_SEXPR;
    new_value->count = 0;
    new_value->cells = NULL;

    return new_value;
}

//----------------------------------------------------------------- FUNCTION ---
/**
 * Frees given LispValue ressources according to its type
 *
 *   -> nothing
 *
 * time  O(?)
 * space O(?)
 */
void delete_lispvalue(LispValue *lispvalue)
{
    switch (lispvalue->type)
    {
        case LVAL_NUMBER:
            break;

        case LVAL_ERR:
            free(lispvalue->error);
            break;

        case LVAL_SYMBOL:
            free(lispvalue->symbol);
            break;

        case LVAL_SEXPR:
            for (int i = 0; i < lispvalue->count; i++)
            {
                delete_lispvalue(lispvalue->cells[i]);
            }
            free(lispvalue->cells);
            break;
    }

    free(lispvalue);
}

//----------------------------------------------------------------- FUNCTION ---
/**
 * Evaluates given AST node of type number
 * 
 *   -> pointer to a LispValue number
 *
 * time  O(?)
 * space O(?)
 */
LispValue* read_lispvalue_number(mpc_ast_t *ast)
{
    errno = 0;
    double number = strtod(ast->contents, NULL);

    return (errno != ERANGE)
        ? new_lispvalue_number(number)
        : new_lispvalue_error("invalid number");
}

//----------------------------------------------------------------- FUNCTION ---
/**
 * Adds a given LispValue to the list of expressions of a given LispValue of 
 * type sexpr
 * 
 *   -> pointer to given LispValue sexpr
 *
 * time  O(?)
 * space O(?)
 */
LispValue* add_lispvalue(LispValue *sexpr, LispValue *lispvalue)
{
    sexpr->count++;
    sexpr->cells = realloc(
        sexpr->cells,
        sizeof(LispValue*) * sexpr->count);
    sexpr->cells[sexpr->count-1] = lispvalue;

    return sexpr;
}

//----------------------------------------------------------------- FUNCTION ---
/**
 * Traverses given AST and evaluates the expression
 *
 *   -> pointer to a LispValue
 *
 * time  O(?)
 * space O(?)
 */
LispValue* read_lispvalue(mpc_ast_t *ast)
{
    if (strstr(ast->tag, "number"))
    {
        return read_lispvalue_number(ast);
    }
    else if (strstr(ast->tag, "symbol"))
    {
        return new_lispvalue_symbol(ast->contents);
    }
    else
    {
        LispValue *lispvalue = NULL;

        if (!(strcmp(ast->tag, ">")) || (strstr(ast->tag, "sexpr")))
        {
            lispvalue = new_lispvalue_sexpr();
        }

        for (int i = 0; i < ast->children_num; i++)
        {
            if (!(strcmp(ast->children[i]->contents, "(")) ||
                !(strcmp(ast->children[i]->contents, ")")) ||
                !(strcmp(ast->children[i]->tag, "regex")))
            {
                continue;
            }
            else
            {
                lispvalue = add_lispvalue(
                    lispvalue, 
                    read_lispvalue(ast->children[i]));
            }
        }
        
        return lispvalue;
    }
}

//----------------------------------------------------------------- Function ---
/**
 * Pretty prints given LispValue of type sexpr
 * 
 *   -> Nothing
 *
 * time  O(?)
 * space O(?)
 */
void print_lispvalue_sexpr(LispValue *lispvalue, char open, char close)
{
    putchar(open);

    for (int i = 0; i < lispvalue->count; i++)
    {
        print_lispvalue(lispvalue->cells[i]);
        putchar(' ');
    }
    
    putchar(close);
}

//----------------------------------------------------------------- Function ---
/**
 * Pretty prints given LispValue according to its type
 * 
 *   -> Nothing
 *
 * time  O(?)
 * space O(?)
 */
void print_lispvalue(LispValue *lispvalue)
{
    switch (lispvalue->type)
    {
    case LVAL_NUMBER:
        printf(FG_YELLOW "%f" RESET, lispvalue->number);
        break;

    case LVAL_ERR:
        printf(FG_RED "Error : %s" RESET, lispvalue->error);
        break;

    case LVAL_SYMBOL:
        printf(FG_CYAN "%s" RESET, lispvalue->symbol);
        break;

    case LVAL_SEXPR:
        print_lispvalue_sexpr(lispvalue, '(', ')');
        break;                
    }
}

//----------------------------------------------------------------- FUNCTION ---
/**
 * Pretty prints given LispValue followed by newline
 *
 *   -> Nothing
 *
 * time  O(?)
 * space O(?)
 */
void print_lispvalue_newline(LispValue *lispvalue)
{
    print_lispvalue(lispvalue);
    putchar('\n');
}


//----------------------------------------------------------------- Function ---
/**
 * Evaluates given LispValue of type sexpr
 * 
 *   -> pointer to result LispValue
 *
 * time  O(?)
 * space O(?)
 */
LispValue* eval_lispvalue_sexpr(LispValue *lispvalue)
{
    /* evaluate children */
    for (int i = 0; i < lispvalue->count; i++)
    {
        lispvalue->cells[i] = eval_lispvalue(lispvalue->cells[i]);
    }

    /* check for errors */
    /* ? can we abort sooner on error as we evaluate children ? */
    for (int i = 0; i < lispvalue->count; i++)
    {
        if (lispvalue->cells[i]->type == LVAL_ERR)
        {
            return take_lispvalue(lispvalue, i);
        }
    }    

    /* empty expression */
    if (lispvalue->count == 0) 
    { 
        return lispvalue;
    }
    /* single expression */
    else if (lispvalue->count == 1) 
    { 
        return take_lispvalue(lispvalue, 0);
    }
    else
    {
        /* make sure first element is a symbol */
        LispValue *first_element = pop_lispvalue(lispvalue, 0);
        if (first_element->type != LVAL_SYMBOL)
        {
            delete_lispvalue(first_element);
            delete_lispvalue(lispvalue);
            return new_lispvalue_error(
                "S-expression does not start with a symbol !");
        }
        else
        {
            LispValue *result = eval_lispvalue_operator(
                lispvalue, 
                first_element->symbol);
            delete_lispvalue(first_element);
            return result;
        }
    }   
}

//----------------------------------------------------------------- Function ---
/**
 * Evaluates given LispValue
 * 
 *   -> pointer to result LispValue
 *
 * time  O(?)
 * space O(?)
 */
LispValue* eval_lispvalue(LispValue *lispvalue)
{
    if (lispvalue->type == LVAL_SEXPR)
    {
        return eval_lispvalue_sexpr(lispvalue);
    }
    else
    {
        return lispvalue;
    }
    
}

//----------------------------------------------------------------- Function ---
/**
 * Extracts single element from given LispValue of type sexpr
 * Shifts the rest of element list backward over extracted element
 * 
 *   -> Extracted LispValue
 *
 * time  O(?)
 * space O(?)
 * 
 * todo
 *   [] make sure you understand why extracted_element still points to something
 */
LispValue* pop_lispvalue(LispValue *lispvalue, int index)
{
    LispValue *extracted_element = lispvalue->cells[index];

    memmove(
        &lispvalue->cells[index],
        &lispvalue->cells[index+1],
        sizeof(LispValue*) * (lispvalue->count - index - 1));

    lispvalue->count--;

    lispvalue->cells = realloc(
        lispvalue->cells,
        sizeof(LispValue*) * lispvalue->count);
    
    return extracted_element;
}

//----------------------------------------------------------------- Function ---
/**
 * Extracts single element from given LispValue of type sexpr
 * Deletes the rest
 * 
 *   -> Extracted LispValue
 *
 * time  O(?)
 * space O(?)
 * 
 * todo
 *   [] make sure you understand why extracted_element still points to something
 */
LispValue* take_lispvalue(LispValue *lispvalue, int index)
{
    LispValue *extracted_element = pop_lispvalue(lispvalue, index);
    delete_lispvalue(lispvalue);

    return extracted_element;
}

/*
 * ---------------------------------------------------------------- Function ---
 * Goes over all elements of given LispValue
 * Checks that they are all of type number
 * 
 *   -> Truth value of predicate
 *
 * time  O(?)
 * space O(?)
 *
 */
int are_all_numbers(LispValue *arguments)
{
    // int result = 1;
    for (int i = 0; i < arguments->count; i++)
    {
        if (arguments->cells[i]->type != LVAL_NUMBER)
        {
            // result = 0;
            return 0;
        }
    }
    return 1;
}

/*
 * ---------------------------------------------------------------- Function ---
 * Performs operation for given operator and LispValue representing all the
 * arguments to operate on
 * 
 *   -> Evaluation result LispValue
 *
 * time  O(?)
 * space O(?)
 *
 */
LispValue* eval_lispvalue_operator(LispValue *arguments, char *operator)
{
    /* make sure all arguments are numbers */
    // for (int i = 0; i < arguments->count; i++)
    // {
    //     if (arguments->cells[i]->type != LVAL_NUMBER)
    //     {
    //         delete_lispvalue(arguments);
    //         return new_lispvalue_error("Cannot operate on non-number !");
    //     }
    // }
    if (!(are_all_numbers(arguments)))
    {
        delete_lispvalue(arguments);
        return new_lispvalue_error("Cannot operate on non-number !");
    }
    else
    {
        LispValue *first_argument = pop_lispvalue(arguments, 0);

        /* unary negation */
        if (!(strcmp(operator, "-")) && arguments->count == 0)
        {
            first_argument->number = -(first_argument->number);
        }

        while(arguments->count > 0)
        {
            LispValue *next_argument = pop_lispvalue(arguments, 0);

            switch (*operator)
            {
                case '+' :
                    first_argument->number += next_argument->number;
                    break;
                
                case '-' :
                    first_argument->number -= next_argument->number;
                    break;

                case '*' :
                    first_argument->number *= next_argument->number;
                    break;

                case '/' :
                    if (next_argument->number == 0)
                    {
                        delete_lispvalue(first_argument);
                        // delete_lispvalue(next_argument);
                        first_argument = new_lispvalue_error(
                            "Division by Zero !");
                    }
                    else
                    {
                        first_argument->number /= next_argument->number;
                    }                    
                    break;

                case '%' :
                    first_argument->number = fmod(
                        first_argument->number,
                        next_argument->number);
                    break;

                case '^' :
                    first_argument->number = pow(
                        first_argument->number,
                        next_argument->number);
                    break;

                case '>' :
                    first_argument->number = 
                        (first_argument->number > next_argument->number)
                            ? first_argument->number
                            : next_argument->number;
                    break;
                    
                case '<' :
                 first_argument->number = 
                        (first_argument->number < next_argument->number)
                            ? first_argument->number
                            : next_argument->number;
                    break;
            }

            delete_lispvalue(next_argument);
        }

    delete_lispvalue(arguments);
    return first_argument;
    }
}

//----------------------------------------------------------------- FUNCTION ---
/**
 * Generates auto-completes matches from global word vocabulary
 *
 *   -> Returns matches from vocabulary
 *
 * time  O(?)
 * space O(?)
 *
 * see https://thoughtbot.com/blog/tab-completion-in-gnu-readline
 */
char* completion_generator(const char *text, int state)
{
    static int match_index, length;
    char *match;

    /*
     * readline calls this function with state = 0 the first time
     * this initialize once for the completion session
     */
    if (!state)
    {
        match_index =0;
        length = strlen(text);
    }

    while ((match = vocabulary[match_index++]))
    {
        if (strncmp(match, text, length) == 0)
        {
            /* readline free() the returned string, correct ? */
            return strdup(match);
        }
    }

    return NULL;
}

//----------------------------------------------------------------- FUNCTION ---
/**
 * Handles custom completion registered to readline global variable
 *
 *   -> Completion matches
 *
 * time  O(?)
 * space O(?)
 */
char** completer(const char *text, int start, int end)
{
    /* not doing filename completion even if 0 matches */
    // rl_attempted_completion_over = 1;

    /* readline expects char** fn(char*, int, int) */
    /* temp workaround compiler warnings for unused-parameters */
    int unused  = start + end;
    unused++;

    return rl_completion_matches(text, &completion_generator);
}

//----------------------------------------------------------------- FUNCTION ---
/**
 * pretty prints a prompt
 *
 *   -> Nothing
 *
 * time  O(?)
 * space O(?)
 */
void print_prompt()
{
    fputs(
        BOLD FG_GREEN "lispy>" RESET,
        stdout);
}

//--------------------------------------------------------------------- MAIN ---
/**
 *
 *   -> Error code
 *
 * time  O(?)
 * space O(?)
 */
int main()
{
    //----------------------------------------------------------- playground
    // LispValue lispValuePlay = create_lispvalue_number(5.0);
    // print_lispvalue_debug(&lispValuePlay);
    // print_lispvalue_newline(&lispValuePlay);
    // print_lispvalue(&lispValuePlay);


    //--------------------------------------------------------------- parser
    mpc_parser_t *number_parser = mpc_new("number");
    mpc_parser_t *symbol_parser = mpc_new("symbol");
    mpc_parser_t *sexpr_parser  = mpc_new("sexpr");
    mpc_parser_t *expr_parser   = mpc_new("expr");
    mpc_parser_t *lispy_parser  = mpc_new("lispy");

    mpca_lang(
        MPCA_LANG_DEFAULT,
        "                                                                      \
        number   : /[-]?[0-9]*[.]?[0-9]+([eE][-+]?[0-9]+)?/ ;                  \
        symbol   : '+' | '-' | '*' | '/' | '%' | '^' | '>' | '<' ;             \
        sexpr    : '(' <expr>* ')' ;                                           \
        expr     : <number> | <symbol> | <sexpr> ;                             \
        lispy    : /^/ <expr>* /$/ ;                                           \
        ",
        number_parser,
        symbol_parser, 
        sexpr_parser, 
        expr_parser, 
        lispy_parser);

    //------------------------------------------------------------ completer
    /* register custom completer with readline global variable */
    rl_attempted_completion_function = &completer;

    //---------------------------------------------------------------- intro
    puts(
        BG_BLUE
        "Lispy version 0.0.0.0.1 " RESET
        FG_BRIGHT_CYAN
        "to Exit press CTRL + C" RESET);


    //----------------------------------------------------------- input loop
    for(;;)
    {
        print_prompt();

        /* readline doesn't like escape codes :/ */
        char* input = readline("");

        if (input != NULL)
        {
            add_history(input);

            /* try to parse input */
            mpc_result_t mpc_result;

            if(mpc_parse("<stdin>", input, lispy_parser, &mpc_result))
            {
                mpc_ast_print(mpc_result.output);


                LispValue *lispvalue_result = read_lispvalue(mpc_result.output);
                print_lispvalue_newline(lispvalue_result);

                lispvalue_result = eval_lispvalue(lispvalue_result);
                print_lispvalue_newline(lispvalue_result);
                delete_lispvalue(lispvalue_result);

                mpc_ast_delete(mpc_result.output);
            }
            else
            {
                mpc_err_print(mpc_result.error);
                mpc_err_delete(mpc_result.error);
            }

            /* readline malloc'd input*/
            free(input);
        }
    }

    //-------------------------------------------------------------- cleanup
    mpc_cleanup(
        5,
        number_parser,
        symbol_parser,
        sexpr_parser,
        expr_parser,
        lispy_parser);

    return 0;
}
