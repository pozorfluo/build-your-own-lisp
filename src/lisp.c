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
    char *pError;
    char *pSymbol;
    /* sexpr */
    int count;
    struct LispValue **pCell;
} LispValue;

//-------------------------------------------------- STATIC GLOBAL VARIABLES ---
/* Readline auto-completion configuration */
static char *vocabulary[] =
{
    "fourcheau",
    "monparounaze",
    "karl",
    "lagerfeld",
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
LispValue* new_lispvalue_number(double x)
{
    LispValue *pLispValue = malloc(sizeof(LispValue));

    pLispValue->type = LVAL_NUMBER;
    pLispValue->number = x;

    return pLispValue;
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
LispValue* new_lispvalue_error(char *pMessage)
{
    LispValue *pLispValue = malloc(sizeof(LispValue));

    pLispValue->type = LVAL_ERR;
    pLispValue->pError = malloc(strlen(pMessage) + 1);
    strcpy(pLispValue->pError, pMessage);

    return pLispValue;
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
LispValue* new_lispvalue_symbol(char *pSymbol)
{
    LispValue *pLispValue = malloc(sizeof(LispValue));

    pLispValue->type = LVAL_SYMBOL;
    pLispValue->pSymbol = malloc(strlen(pSymbol) + 1);
    strcpy(pLispValue->pSymbol, pSymbol);

    return pLispValue;
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
    LispValue *pLispValue = malloc(sizeof(LispValue));

    pLispValue->type = LVAL_SEXPR;
    pLispValue->count = 0;
    pLispValue->pCell = NULL;

    return pLispValue;
}

//----------------------------------------------------------------- Function ---
/**
 * Frees given LispValue ressources according to its type
 *
 *   -> nothing
 *
 * time  O(?)
 * space O(?)
 */
void delete_lispvalue(LispValue *pLispValue)
{
    switch (pLispValue->type)
    {
        case LVAL_NUMBER:
            break;

        case LVAL_ERR:
            free(pLispValue->pError);
            break;

        case LVAL_SYMBOL:
            free(pLispValue->pSymbol);
            break;

        case LVAL_SEXPR:
            for (int i = 0; i < pLispValue->count; i++)
            {
                delete_lispvalue(pLispValue->pCell[i]);
            }
            free(pLispValue->pCell);
            break;
    }

    free(pLispValue);
}

//----------------------------------------------------------------- Function ---
/**
 * Evaluates given AST node of type number
 * 
 *   -> pointer to a LispValue number
 *
 * time  O(?)
 * space O(?)
 */
LispValue* read_lispvalue_number(mpc_ast_t *pAST)
{
    errno = 0;
    double number = strtod(pAST->contents, NULL);

    return (errno != ERANGE)
        ? new_lispvalue_number(number)
        : new_lispvalue_error("invalid number");
}

//----------------------------------------------------------------- Function ---
/**
 * Adds a given LispValue to the list of expressions of a given LispValue of 
 * type sexpr
 * 
 *   -> pointer to given LispValue sexpr
 *
 * time  O(?)
 * space O(?)
 */
LispValue* add_lispvalue(
    LispValue *pLispValueSexpr,
    LispValue *pLispValue)
{
    pLispValueSexpr->count++;
    pLispValueSexpr->pCell = realloc(
        pLispValueSexpr->pCell,
        sizeof(LispValue*) * pLispValueSexpr->count);
    pLispValueSexpr->pCell[pLispValueSexpr->count-1] = pLispValue;

    return pLispValueSexpr;
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
LispValue* read_lispvalue(mpc_ast_t *pAST)
{
    if (strstr(pAST->tag, "number"))
    {
        return read_lispvalue_number(pAST);
    }
    else 
    {
        LispValue *pLispValue = NULL;

        if (!(strcmp(pAST->tag, ">")) || (strstr(pAST->tag, "sexpr")))
        {
            pLispValue = new_lispvalue_sexpr();
        }

        for (int i = 0; i < pAST->children_num; i++)
        {
            if (!(strcmp(pAST->children[i]->contents, "(")) ||
                !(strcmp(pAST->children[i]->contents, ")")) ||
                !(strcmp(pAST->children[i]->tag, "regex")))
            {
                continue;
            }
            else
            {
                pLispValue = add_lispvalue(
                    pLispValue, 
                    read_lispvalue(pAST->children[i]));
            }
        }
        
        return pLispValue;
    }
}


//----------------------------------------------------------------- FUNCTION ---
/**
 * Pretty prints a dump of all fields of a given LispValue
 *
 *   -> nothing
 *
 * time  O(?)
 * space O(?)
 */
// void print_lispvalue_debug(const LispValue *v)
// {
//     printf(
//         FG_YELLOW
//         "LispValue.type   = %d\n"
//         "LispValue.number = %f\n"
//         "LispValue.error  = %d\n"
//         RESET,
//         v->type, v->number, v->pError);
// }


//----------------------------------------------------------------- FUNCTION ---
/**
 * pretty prints a given LispValue followed by newline
 *
 *   -> Nothing
 *
 * time  O(?)
 * space O(?)
 */
// void print_lispvalue_newline(const LispValue *v)
// {
//     print_lispvalue(v);
//     putchar('\n');
// }

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
    static int matchIndex, length;
    char *match;

    /*
     * readline calls this function with state = 0 the first time
     * this initialize once for the completion session
     */
    if (!state)
    {
        matchIndex =0;
        length = strlen(text);
    }

    while ((match = vocabulary[matchIndex++]))
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
 * Performs operation for given operator string and numbers
 *
 *   -> Evaluation result
 *
 * time  O(?)
 * space O(?)
 *
 * todo
 *   [] process the alternate add, sub, mul, div, mod, pow, min, max form
 */
// LispValue mpc_ast_eval_op(
//     const LispValue *x,
//     const char *operator,
//     const LispValue *y)
// {
//     if (x->type == LVAL_ERR) { return *x; }
//     if (y->type == LVAL_ERR) { return *y; }

//     switch(*operator)
//     {
//         case '+' :
//             return create_lispvalue_number(x->number + y->number);
//         case '-' :
//             return create_lispvalue_number(x->number - y->number);
//         case '*' :
//             return create_lispvalue_number(x->number * y->number);
//         case '/' :
//             return (y->number == 0)
//                 ? create_lispvalue_err(LERR_DIV_ZERO)
//                 : create_lispvalue_number(x->number / y->number);
//         case '%' :
//             return create_lispvalue_number(fmod(x->number, y->number));
//         case '^' :
//             return create_lispvalue_number(pow(x->number, y->number));
//         case '>' :
//             return (x->number > y->number) ? *x : *y;
//         case '<' :
//             return (x->number < y->number) ? *x : *y;
//     }

//     /* if given operator is not supported */
//     return create_lispvalue_number(LERR_BAD_OP);
// }

//----------------------------------------------------------------- FUNCTION ---
/**
 * Traverses AST and evaluates the expression
 *
 *   -> Evaluation
 *
 * time  O(?)
 * space O(?)
 */
// LispValue mpc_ast_eval(mpc_ast_t *ast)
// {
//     /* return numbers directly */
//     if (strstr(ast->tag, "number"))
//     {
//         errno = 0;
//         double x = strtod(ast->contents, NULL);
//         return (errno != ERANGE)
//             ? create_lispvalue_number(x)
//             : create_lispvalue_err(LERR_BAD_NUM);
//     }

//     /* operator is always expected to be the second child */
//     char *operator = ast->children[1]->contents;

//     LispValue v = mpc_ast_eval(ast->children[2]);
//     LispValue y;

//     /* iterate over remaining children */
//     int i = 3;
//     while (strstr(ast->children[i]->tag, "expr"))
//     {
//         y = mpc_ast_eval(ast->children[i]);
//         v = mpc_ast_eval_op(&v, operator, &y);
//         i++;
//     }

//     return v;
// }

//----------------------------------------------------------------- Function ---
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
    mpc_parser_t *pNumberParser = mpc_new("number");
    mpc_parser_t *pSymbolParser = mpc_new("symbol");
    mpc_parser_t *pSexprParser  = mpc_new("sexpr");
    mpc_parser_t *pExprParser   = mpc_new("expr");
    mpc_parser_t *pLispyParser  = mpc_new("lispy");

    mpca_lang(
        MPCA_LANG_DEFAULT,
        "                                                                      \
        number   : /[-]?[0-9]+([.]?[0-9]+([eE][-+]?[0-9]+)?)?/ ;               \
        symbol   : '+' | '-' | '*' | '/' | '%' | '^' | '>' | '<' | ;           \
        sexpr    : '(' <expr>* ')' ;                                           \
        expr     : <number> | <symbol> | <sexpr> ;                             \
        lispy    : /^/ <expr>* /$/ ;                                           \
        ",
        pNumberParser, pSymbolParser, pSexprParser, pExprParser, pLispyParser);

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
            mpc_result_t mpcResult;

            if(mpc_parse("<stdin>", input, pLispyParser, &mpcResult))
            {
                mpc_ast_print(mpcResult.output);

                // LispValue LispValueResult = mpc_ast_eval(mpcResult.output);
                // print_lispvalue_newline(&LispValueResult);
                // print_lispvalue_debug(&LispValueResult);

                mpc_ast_delete(mpcResult.output);
            }
            else
            {
                mpc_err_print(mpcResult.error);
                mpc_err_delete(mpcResult.error);
            }

            /* readline malloc'd input*/
            free(input);
        }
    }

    //-------------------------------------------------------------- cleanup
    mpc_cleanup(
        5,
        pNumberParser,
        pSymbolParser,
        pSexprParser,
        pExprParser,
        pLispyParser);

    return 0;
}
