#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <editline/readline.h>
#include <editline/history.h>

#include "../include/mpc.h"

/*
 * --------------------------------------------------------- PLATFORM MACROS ---
 * see https://sourceforge.net/p/predef/wiki/OperatingSystems/
 */

// #ifdef _WIN32

// #elif defined __gnu_linux__

// #elif defined __APPLE__

// #endif

/*
 * ----------------------------------------------------------- MAGIC NUMBERS ---
 */



/*
 * ------------------------------------------------------------- ANSI MACROS ---
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


/*
 * ------------------------------------------------------------ DEBUG MACROS ---
 */

/*
 * ------------------------------------------------- Static Global Variables ---
 */

/* Readline auto-completion configuration */
    static char *vocabulary[] = {
        "fourcheau",
        "monparounaze",
        "karl",
        "lagerfeld",
        NULL
    };

/*
 * ---------------------------------------------------------------- Function ---
 * Generates auto-completes matches from global word vocabulary
 * 
 *   -> Returns matches from vocabulary
 *
 * time  O(?)
 * space O(?)
 * 
 * see https://thoughtbot.com/blog/tab-completion-in-gnu-readline
 */
char* completion_generator(const char *text, int state) {

    static int matchIndex, length;
    char *match;

    /* 
     * readline calls this function with state = 0 the first time
     * this initialize once for the completion session
     */
    if (!state) {
        matchIndex =0;
        length = strlen(text);
    }

    while ((match = vocabulary[matchIndex++])) {
        if (strncmp(match, text, length) == 0) {
            /* readline free() the returned string, correct ? */
            return strdup(match);
        }
    }

    return NULL;
}

/*
 * ---------------------------------------------------------------- Function ---
 * Handles custom completion registered to readline global variable
 * 
 *   -> Completion matches
 *
 * time  O(?)
 * space O(?)
 */
char** completer(const char *text, int start, int end) {

    /* not doing filename completion even if 0 matches */
    // rl_attempted_completion_over = 1;

    /* readline expects char** fn(char*, int, int) */
    /* temp workaround compiler warnings for unused-parameters */
   int unused  = start + end;
   unused++;
    
    return rl_completion_matches(text, &completion_generator);
}


/*
 * ---------------------------------------------------------------- Function ---
 * Performs operation for given operator string and numbers
 * 
 *   -> Evaluation
 *
 * time  O(?)
 * space O(?)
 * 
 * todo 
 *   [] process the alternate add, sub, mul, div, mod, pow, min, max form
 */
double mpc_ast_eval_op(double x, char *operator, double y) {

    switch(*operator) {
        case '+' :
            return x + y;
        case '-' :
            return x - y;
        case '*' :
            return x * y;
        case '/' :
            return x / y;
        case '%' :
            return fmod(x, y);
        case '^' :
            return pow(x, y);
        case '>' :
            return (x > y) ? x : y;
        case '<' :
            return (x < y) ? x : y;

    }

    return 0.0;
}

/*
 * ---------------------------------------------------------------- Function ---
 * Traverses AST and evaluates the expression
 * 
 *   -> Evaluation
 *
 * time  O(?)
 * space O(?)
 */
double mpc_ast_eval(mpc_ast_t *ast) {

    /* return numbers directly */
    if (strstr(ast->tag, "number")) {
        return atof(ast->contents);
    }

    /* operator is always expected to be the second child */
    char *operator = ast->children[1]->contents;

    /* evaluate expression in third child */
    double x = mpc_ast_eval(ast->children[2]);

    /* iterate over remaining children */
    int i = 3;
    while (strstr(ast->children[i]->tag, "expr")) {
        x = mpc_ast_eval_op(x, operator, mpc_ast_eval(ast->children[i]));
        i++;
    }

    return x;
}

/*
 * ---------------------------------------------------------------- Function ---
 * Traverses AST and counts the nodes
 * 
 *   -> Number of nodes
 *
 * time  O(n)
 * space O(?)
 */
int mpc_ast_count_nodes(mpc_ast_t *ast) {

    /* base case */
    if (ast->children_num == 0) {
         return 1; 
    }
    /* recursive case */
    else if (ast->children_num >= 1) {
        int total = 1;
        for (int i = 0; i < ast->children_num; i++)
        {
            total += mpc_ast_count_nodes(ast->children[i]);
        }
        return total;
    }
    else {
        return 0;
    }
}

/*
 * ---------------------------------------------------------------- Function ---
 * Traverses AST and counts the leaves
 * 
 *   -> Number of leaves
 *
 * time  O(n)
 * space O(?)
 */
int mpc_ast_count_leaves(mpc_ast_t *ast) {

    /* base case */
    if (ast->children_num == 0) {
         return 1; 
    }
    /* recursive case */
    else if (ast->children_num >= 1) {
        int total = 0;
        for (int i = 0; i < ast->children_num; i++)
        {
            total += mpc_ast_count_leaves(ast->children[i]);
        }
        return total;
    }
    else {
        return 0;
    }
}

/*
 * ---------------------------------------------------------------- Function ---
 * Traverses AST and counts the branches
 * 
 * we assume the goal of the exercice was to count the number of times
 * the AST splits
 * 
 *   -> Number of branches
 *
 * time  O(n)
 * space O(?)
 */
int mpc_ast_count_branches(mpc_ast_t *ast) {

    /* base case */
    if (ast->children_num == 0) {
         return 0; 
    }
    /* recursive case */
    else if (ast->children_num >= 1) {
        int total = ast->children_num;
        for (int i = 0; i < ast->children_num; i++)
        {
            total += mpc_ast_count_branches(ast->children[i]);
        }
        return total;
    }
    else {
        return 0;
    }
}

/*
 * -------------------------------------------------------------------- main ---
 * Is the program entry point
 * 
 *   -> Error code
 *
 * time  O(?)
 * space O(?)
 */
int main()
{
    /*
     * ---------------------------------------------------------- parser ---
     */
    /* create parsers */
    mpc_parser_t *parserNumber   = mpc_new("number");
    mpc_parser_t *parserOperator = mpc_new("operator");
    mpc_parser_t *parserExpr     = mpc_new("expr");
    mpc_parser_t *parserLispy    = mpc_new("lispy");

    /* define parsers language */
    mpca_lang(MPCA_LANG_DEFAULT,
    "                                                                       \
      number   : /[-]?[0-9]+([.]?[0-9]+([eE][-+]?[0-9]+)?)?/;               \
      operator : '+' | '-' | '*' | '/' | '%' | '^' | '>' | '<' |            \
                 \"add\" | \"sub\" | \"mul\" | \"div\" | \"mod\" | \"pow\"; \
      expr     : <number> | '(' <operator> <expr>+ ')' ;                    \
      lispy    : /^/ <operator> <expr>+ /$/;                                \
    ",
    parserNumber, parserOperator, parserExpr, parserLispy);

    /*
     * ------------------------------------------------------- completer ---
     */
    /* register custom completer with readline global variable */
    rl_attempted_completion_function = &completer;

    /*
     * ----------------------------------------------------------- intro ---
     */
    /* print version and some instructions */
    puts("Lispy version 0.0.0.0.1");
    puts("to Exit press CTRL + C");


    /*
     * ------------------------------------------------------------ loop ---
     */
    for(;;){
        /* print prompt */
        fputs(BOLD FG_GREEN "lispy>" RESET, 
              stdout);

        /* get input*/
        /* readline doesn't like escape codes :/ */
        char* input = readline("");
        
        if (input != NULL){
            /* add input to history */
            add_history(input);

            /* echo input back */
            printf(BOLD FG_BRIGHT_RED"No you a %s\n" RESET,
                input);

            /* try to parse input */
            mpc_result_t mpcResult;

            if(mpc_parse("<stdin>", input, parserLispy, &mpcResult)) {
                /* print the AST */
                mpc_ast_print(mpcResult.output);

                /* get AST from output */
                mpc_ast_t *ast = mpcResult.output;
                printf(FG_YELLOW "Number of nodes    : %i\n" RESET, 
                       mpc_ast_count_nodes(ast));
                printf(FG_YELLOW "Number of leaves   : %i\n" RESET, 
                       mpc_ast_count_leaves(ast));
                printf(FG_YELLOW "Number of branches : %i\n" RESET, 
                       mpc_ast_count_branches(ast));
                printf("Tag                : %s\n", ast->tag);
                printf("Contents           : %s\n", ast->contents);
                printf("Number of children : %i\n", ast->children_num);

                /* get first child */
                mpc_ast_t *astC0 = ast->children[0];
                printf("C0 Tag                : %s\n", astC0->tag);
                printf("C0 Contents           : %s\n", astC0->contents);
                printf("C0 Number of children : %i\n", astC0->children_num);

                /* print AST evaluation */
                printf(BOLD FG_BRIGHT_RED "-> Eval : %f\n" RESET,
                       mpc_ast_eval(ast));

                mpc_ast_delete(mpcResult.output);
            }
            else {
                /* print the error */
                mpc_err_print(mpcResult.error);
                mpc_err_delete(mpcResult.error);
            }

            /* free readline malloc'd input*/
            free(input);
        }
    }

    /*
     * --------------------------------------------------------- cleanup ---
     */
    /* undefine and delete parsers */
    mpc_cleanup(4, parserNumber, parserOperator, parserExpr, parserLispy);

    return 0;
}

/* 
 * Write a regular expression matching strings of all a or b such as aababa or bbaa.
 *   \b[ab]+\b
 * 
 * Write a regular expression matching strings of consecutive a and b such as ababab or aba.
 *   \b(?<pattern>ab)((\k<pattern>)|(?:a))+\b
 * 
 * Write a regular expression matching pit, pot and respite but not peat, spit, or part.
 *   \b(?:pit)|\b(?:pot)|\b(?:respite)
 */