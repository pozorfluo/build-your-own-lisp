#!/usr/bin/env python
"""\x1B[32;7m
┌──────────────────────────────────────────────────────────────────────────────┐
│   primality.py                                                               │
└──────────────────────────────────────────────────────────────────────────────┘
\x1B[0;32;49m
███ Usage:
    primality.py test <integer>...
    primality.py prime <from> <to> [options]
    primality.py mersenne [number|exponent] <n> [options]

███ Options:
    -h, --help           Show this screen.
    --version            Show version.
    --separator=<string> Specify a separator [default : ", "].

███ Hints : 
    Use --separator=$'\\t' to get a tab as separator.

\x1B[90m
    Explore prime number and in particular prime numbers that are also (2^n - 1)
    
    Test primality of given integers list
    List prime numbers, mersenne numbers, known mersenne prime exponents
\x1B[0m
"""
import sys
from docopt import docopt

mersenne_exponents = [
    2,
    3,
    5,
    7,
    13,
    17,
    19,
    31,
    61,
    89,
    107,
    127,
    521,
    607,
    1279,
    2203,
    2281,
    3217,
    4253,
    4423,
    9689,
    9941,
    11213,
    19937,
    21701,
    23209,
    44497,
    86243,
    110503,
    132049,
    216091,
    756839,
    859433,
    1257787,
    1398269,
    2976221,
    3021377,
    6972593,
    13466917,
    20996011,
    24036583,
    25964951,
    30402457,
    32582657,
    37156667,
    42643801,
    43112609,
    57885161,
    74207281,
    77232917,
    82589933,
]


def mersenne_number_generator(count):
    for n in range(count):
        yield (2 ** n) - 1

def is_prime(integer):
    if integer <= 3:
        return integer > 1
    elif (integer % 2) == 0 or (integer % 3) == 0:
        return False

    for i in [i for i in range(5, integer, 6) if (i * i) < integer]:
        if (integer % i) == 0 or (integer % (i + 2)) == 0:
            return False
    return True


# --------------------------------------------------------------------- main ---
def main() -> None:
    """
    docopt based CLI
     
    See : http://docopt.org/
    """
    arguments = docopt(__doc__, version="0.1")

    if arguments["--separator"] is None:
        arguments["--separator"] = ", "

    separator = arguments["--separator"]
    # ----------------------------------------------------- prime command
    if arguments["prime"]:
        try:
            start = int(arguments["<from>"])
            end = int(arguments["<to>"])
        except ValueError:
            print("Error : <from> and <to> must be integers !")
            sys.exit(0)

        for n in range(start, end):
            if is_prime(n):
                print(n, end=separator, flush=True)
    # -------------------------------------------------- mersenne command
    elif arguments["mersenne"]:
        try:
            n = int(arguments["<n>"])
        except ValueError:
            print("Error : <n> must be an integer !")
            sys.exit(0)
        # ------------------------------- mersenne number subcommand
        if arguments["number"]:
            for m in mersenne_number_generator(n):
                print(m, end=separator, flush=True)
        # -------------------------------- mersenne prime subcommand
        else:
            for exp in [e for i, e in enumerate(mersenne_exponents) if i <= n]:
                # --------------------- mersenne exponent subcommand
                if arguments["exponent"]:
                    print(f"{exp}", end=separator, flush=True)
                else:
                    print(f"2**{exp} - 1", end=separator, flush=True)
    # ----------------------------------------------------- test command
    elif arguments["test"]:
        try:
            for i in arguments["<integer>"]:
                result = "is" if is_prime(int(i)) else "is NOT"
                print(f"{i} {result} prime.")
        except ValueError:
            print(f'Error : expected integer, got "{i}" !')
            sys.exit(0)

    print("\n\nDone !\n")
    # print(arguments)


if __name__ == "__main__":
    main()

