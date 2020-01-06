#include <stdio.h>
#include <string.h>

/*
 * ----------------------------------------------------------- MAGIC NUMBERS ---
 */
#define BUFFER_SIZE 2048

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
static char inputBuffer[BUFFER_SIZE];

/*
 * -------------------------------------------------------------------- main ---
 * is the program entry point
 * 
 *   -> error code
 *
 * time  O(n)
 * space O(1)
 */
int main()
{
    /* print version and some instructions */
    puts("Lispy version 0.0.0.0.1");
    puts("to Exit press CTRL + C");

    for(;;){
        /* print prompt */
        fputs(FG_GREEN "lispy> " RESET, 
              stdout);

        /* read a line of user input up to max buffer size */
        fgets(inputBuffer, BUFFER_SIZE, stdin);

        /* echo input back */
        printf(BOLD FG_BRIGHT_RED"No you a %s" RESET,
               inputBuffer);
    }

    return 0;
}

