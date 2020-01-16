#include <stdio.h>

/*
 * ----------------------------------------------------------- MAGIC NUMBERS ---
 */

/*
 * ------------------------------------------------------------- DEBUG MACRO ---
 */

/*
 * ----------------------------------------------------------------- Function ---
 * prints a Hello World style message a given number of times
 * 
 *     -> void
 *
 * time  O(n)
 * space O(1)
 */
void say_hello_world(int numberOfTimes) {
    for (int i = 0; i < numberOfTimes; i++) {
        printf("say_hello_world() : La Bonne Année avec la bonne CC !\n");
    } 
}

/*
 * -------------------------------------------------------------------- main ---
 * is the program entry point
 * 
 *     -> error code
 *
 * time  O(1)
 * space O(1)
 */

int main() {
    int numberOfTimes = 5;
    int i;
    /* Use a for loop to print out Hello World! five times. */
    for (i = 0; i < 5; i++) {
        printf("for : La Bonne Année avec la bonne CC !\n");
    }

    /* Use a while loop to print out Hello World! five times. */
    i = 0;

    while (i < numberOfTimes)
    {
        puts("while : La Bonne Année avec la bonne CC !");
        i++;
    }
    
    /* Declare a function that outputs Hello World! n number of times. Call this from main. */
    say_hello_world(numberOfTimes);
    
    return 0;
}