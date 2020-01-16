#include <stdio.h>
#include <string.h>

/*
 * ----------------------------------------------------------- MAGIC NUMBERS ---
 */

/*
 * ------------------------------------------------------------- DEBUG MACRO ---
 */



/*
 * ---------------------------------------------------------------- Function ---
 * solves fizzbuzz style exercice for given range
 * 
 *     -> void
 *
 * time  O(n)
 * space O(1)
 */
void solve_fizzbuzz(int range) {
    /* initialize with "fizzbuzz" to avoid guessing the size ? */
    char stringToPrint[] = "fizzbuzz";
    
    for (int i = 1; i <= range; i++) {
        // strcpy(stringToPrint, "");
        memset(stringToPrint, '\0', sizeof(stringToPrint));

        if ((i % 3)==0) {
            strcat(stringToPrint, "fizz");
        }

        if ((i % 5)==0) {
            strcat(stringToPrint, "buzz");
        }

        if(strlen(stringToPrint)==0) {
            /* might as well print directly */
            // sprintf(stringToPrint, "%d", i); 
            printf("%d\n", i); 
        }
        else {
            puts(stringToPrint);
        }

        
    }
}

/*
 * ---------------------------------------------------------------- Function ---
 * solves fizzbuzz style exercice for given range
 * branchless
 * 
 * prints from look up table
 *   NOT(remainder) multiplied by relevant index
 *   
 *     -> void
 *
 * time  O(n)
 * space O(1)
 */
void solve_fizzbuzz_branchless(int range) {
    /* use a table instead of branches */
    const char *stringsToPrint[] = {
        "%d\n",
        "fizz\n",
        // stringsToPrint[3] + 4, //  possible ?
        "buzz\n",
        "fizzbuzz\n" 
    };

    for (int i = 1; i <= range; i++) {
        // printf(stringsToPrint[((i % 3) == 0) + ((i % 5) == 0 ) * 2], i);
        printf(stringsToPrint[!(i % 3) + !(i % 5) * 2], i);
    }
}

/*
 * ---------------------------------------------------------------- Function ---
 * solves fizzbuzz style exercice for given range
 * branchless
 * silly 
 * does not meet the requirements, the number is always printed 
 *   
 *     -> void
 *
 * time  O(n)
 * space O(1)
 */
void solve_fizzbuzz_silly(int range) {
    /*  */
    const char stringsToPrint[] = {
        'f', 'i', 'z', 'z', 'b', 'u', 'z', 'z', '\0', '%', 'd', '\0'
    };

    int index, length;

    for (int i = 1; i <= range; i++) {
        
        index = !((i % 3) == 0) * !(i % 5) * 4;
        length = (!(i % 3) * 4 + !(i % 5) * 4);
        
        // printf("%d %d \n", index, length);
        /* see https://stackoverflow.com/questions/256218/the-simplest-way-of-printing-a-portion-of-a-char-in-c */
        printf("%d %.*s\n",i , length, stringsToPrint + index);
    }
}


/*
 * -------------------------------------------------------------------- main ---
 * is the program entry point
 * 
 *     -> error code
 *
 * time  O(n)
 * space O(1)
 */
int main() {//int argc, char const *argv[]) {

    // solve_fizzbuzz(100);
    // solve_fizzbuzz_branchless(10000);
    solve_fizzbuzz_silly(100);
    return 0;
}
