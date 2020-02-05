#!/usr/bin/env python
"""\x1B[32;7m
┌──────────────────────────────────────────────────────────────────────┐
│   %(scriptName)s                                                      │
└──────────────────────────────────────────────────────────────────────┘\x1B[0;92;49m

███ Usage:
	%(scriptName)s <filepath>
────────────────────────────────────────────────────────────────────────
Read handwritten tests defined in a given file

  Define a test input with a line that is a not lisp style comment
    ( i.e., NOT starting with ";;" )
      
  Define test expected results with the following lines as lisp style comments
      ( i.e., starting with ";;" )

  Empty lines are not skipped !
  Multilines definition are not supported :
    Each 'lisp style comment' line will generate an expect() statement
────────────────────────────────────────────────────────────────────────\x1B[90m 
> example : test.lisp -> define 4 tests 

+ 1 (* 7 5) 3
;; 39
join (head {(+ 1 2) (+ 10 20)}) {5 6 7}
;; {(+ 1.000000 2.000000 ) 5.000000 6.000000 7.000000 }
list 1 2 3 4
;; {1.000000 2.000000 3.000000 4.000000 }
tail (len {+ 5 5})
;; Error :  Function 'tail' passed incorrect type for element 0 !
;;          expected Q-Expression, got Number.
"""
import sys
from collections import namedtuple
import re

# --------------------------------------------- escaped ansi escape sequence ---
# Brackets are escaped to be used with .expect(regexp)
reset = "\x1b[0m"
reverse = "\x1b[7m"
fg_black = "\x1b[30m"
fg_red = "\x1b[31m"
fg_green = "\x1b[32m"
fg_yellow = "\x1b[33m"
fg_cyan = "\x1b[36m"
bg_green = "\x1b[102m"

# --------------------------------------------- handwritten tests definition ---
Test = namedtuple("Test", "input result")

token_specs = [
    ("symbol", r"[a-zA-Z_+\-*%^\/\\=<>!&]+"),
    ("number", r"[-]?[0-9]+[.]?[0-9]*([eE][-+]?[0-9]+)?"),
]
regexp = re.compile("|".join("(?P<%s>%s)" % pair for pair in token_specs))

def pretty_replace(matches):
    if matches.group('number') is None:
        return f"{fg_cyan}{matches.group('symbol')}{reset}"
    else:
        return f"{fg_yellow}{matches.group('number')}{reset}"


def prettify(line):
    """Adorn a test expected result definition line with ansi esc code
    If lispy cosmetics change, this need to be changed accordingly
    """
    if "Error : " in line:
        line = line.replace("Error : ", f"{fg_red}{reverse}Error : {reset}{fg_red}", 1)
    elif (line[:4] == ";;  ") or (line[:4] == ";; \t"):
        line = line.rstrip("\n") + f"{reset}"
    elif ";; <" in line:
        line = line.replace(";; <", f";; {fg_green}<", 1).rstrip("\n") + f"{reset}"
    else:
        line = regexp.sub(pretty_replace, line)
    return line.lstrip(";; ").rstrip("\n")


def read_tests(filepath):
    """Read file containing handwritten test definitions
    
    For each line in file
        If it is not a comment it is a test input definition
            If it is not the first test input definition it is a new test definition
        Else it is a test result definition
            Adorn test result definition with ansi esc code
    
        -> list of Test
    """
    tests = []

    with open(filepath, "r") as testfile:
        test_input = None
        test_result = None

        for line in testfile:
            if line[:2] != ";;":
                if test_input is not None:
                    tests.append(Test(test_input, test_result))
                test_input = line.rstrip("\n")
                test_result = []
            else:
                try:
                    test_result.append(prettify(line))
                except AttributeError:
                    print(
                        "Test must start with an input definition."
                        "Leave a blank line to start a new test with no input."
                    )
        tests.append(Test(test_input, test_result))
    return tests


# --------------------------------------------------------------------- main ---
# todo
# - [x] read handwritten test from separate, easy to edit file
# - [ ] append generated tests to this dict
# - [ ] run test loop once
#
# - [x] work from known/last good build output to generate test
# 		- [x] support Test(input, n lines result)
#
# - [x] define separate decoration rule once
# - [ ] control the flow with func dict as with builtin_operators
# - [ ] key into flow dict with dict.get(key, defaultvalue)
def main() -> None:
    if len(sys.argv) < 2:
        print(__doc__ % {"scriptName": sys.argv[0].split("/")[-1]})
        sys.exit(0)

    test_filepath = sys.argv[1]  # "tests/test.lisp"

    tests = read_tests(test_filepath)

    with open("tests/tests_translated.lisp", "w") as translatedfile:
        for i, test in enumerate(tests):
            translatedfile.write(test.input + "\n")
            print(test.input)
            for j, line in enumerate(test.result):
                translatedfile.write(line + "\n")
                print(line)


if __name__ == "__main__":
    main()
