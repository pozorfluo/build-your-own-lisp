#!/usr/bin/env python
"""\x1B[32;7m
 ┌────────────────────────────────────────────────────────────────────────────┐ 
 │   tests.py                                                                 │ 
 └────────────────────────────────────────────────────────────────────────────┘ 
\x1B[0;32;49m
███ Usage:
	tests.py  <binary> [--test=<file>] [--log=<file>]
					   [--stress=<n>] [--delay=<s>] 
					   [-v | --verbose] [-o | --output]
	tests.py  -h | --help
	tests.py  --version

███ Options:
	-h, --help     Show this screen.
	--version      Show version.
	--test=<file>  Path to handwritten tests definition file [default: tests/test.lisp].
	--log=<file>   Path to log file [default: test_report.yaml].
	--stress=<n>   Number of iterations for Lispenv test [default: 10].
	--delay=<s>    Delay before sending commands [default: 0.05 seconds].
	-v, --verbose  Pretty prints test results in terminal.
	-o, --output   Show spawned process output in terminal.
\x1B[0m 
Run handwritten tests defined in a given file.

  Define a test input with a line that is a not lisp style comment.
    ( i.e., NOT starting with ";;" )
      
  Define test expected results with the following lines as lisp style comments.
      ( i.e., starting with ";;" )

  Empty lines are not skipped !
  Multilines definition are not supported :
    Each 'lisp style comment' line will generate an expect_exact() statement.
\x1B[90m 
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
────────────────────────────────────────────────────────────────────────────────

\x1B[0m
"""
from docopt import docopt
import pexpect

import sys
import os

import random
import string
import datetime

from collections import namedtuple
import re

# --------------------------------------------- escaped ansi escape sequence ---
reset = "\x1b[0m"
reverse = "\x1b[7m"
fg_black = "\x1b[30m"
fg_red = "\x1b[31m"
fg_green = "\x1b[32m"
fg_yellow = "\x1b[33m"
fg_blue = "\x1b[34m"
fg_magenta = "\x1b[35m"
fg_cyan = "\x1b[36m"
fg_white = "\x1b[37m"
fg_default = "\x1b[39m"
bg_green = "\x1b[102m"

# use dict form to expand template string
ansiseq = {
    "reset": reset,
    "reverse": reverse,
    "fg_black": fg_black,
    "fg_red": fg_red,
    "fg_green": fg_green,
    "fg_yellow": fg_yellow,
    "fg_blue": fg_blue,
    "fg_magenta": fg_magenta,
    "fg_cyan": fg_cyan,
    "fg_white": fg_white,
    "fg_default": fg_default,
    "bg_green": bg_green,
}


# ----------------------------------------- Generated tests helper functions ---
# import functools
# @functools.wraps(func)

# def format(op)


def lisp(operatorfunc):
    """Makes the basic operator functions variadic

		The process is interrupted if any intermediate result is a string
			-> result or error string
	"""

    def wrapper(*numbers, **kwargs):
        print(f"lisp: calling {operatorfunc.__name__}() with {numbers}, {kwargs}")
        result = numbers[0]
        for number in numbers[1:]:
            print(f"{operatorfunc.__name__}({result}, {number})")
            result = operatorfunc(result, number)
            if isinstance(result, str):
                return result
        return result

    return wrapper


@lisp
def op_add(a, b):
    return a + b


@lisp
def op_sub(a, b):
    return a - b


@lisp
def op_mul(a, b):
    return a * b


@lisp
def op_div(a, b):
    # Error :  Division by Zero !
    # FG_RED REVERSE "Error : " RESET FG_RED " %s" RESET
    if b == 0:
        return f"{{{fg_red}{reverse} Division by Zero !{reset}}}"
    else:
        return a / b


@lisp
def op_mod(a, b):
    if b == 0:
        return "-nan"
    else:
        return a % b


@lisp
def op_pow(a, b):
    return a ** b


# python max and min are already variadic
op_max = max
op_min = min

# easier to define as a dict; allows access by key
builtins = {
    "add": op_add,
    "sub": op_sub,
    "mul": op_mul,
    "div": op_div,
    "mod": op_mod,
    "pow": op_pow,
    "max": op_max,
    "min": op_min,
    "+": op_add,
    "-": op_sub,
    "*": op_mul,
    "/": op_div,
    "%": op_mod,
    "^": op_pow,
    ">": op_max,
    "<": op_min,
}


def random_symbol(length):
    return "".join(random.choice(string.ascii_letters) for i in range(length))


def operand_generator(count, integer=False):
    for n in range(count):
        bounds = random.getrandbits(32)
        random_value = random.uniform(-bounds, bounds)
        if integer:
            random_value = int(random_value)
        yield (random_symbol(random.getrandbits(4) + 2), random_value)


def is_number(operand):
    try:
        float(operand)
        return True
    except ValueError:
        return False


def qexpr_template(operands):
    qexpr = ["{"]
    for operand in operands:
        if is_number(operand):
            qexpr.append(f"${{fg_yellow}}{operand}${{reset}} ")
        else:
            qexpr.append(f"${{fg_cyan}}{operand}${{reset}} ")
    qexpr.append("}")
    return "".join(qexpr)


qexpr_template("list 1 2 3 4".split(" "))
qexpr_template("{(+ 1.000000 2.000000 ) }".split(" "))
# def sexpr(operatorfunc):
# 	"""Generates a test expected result template string for simple sexpr
# 	"""
# 	def wrapper(*operands):
# 		result = operands[0]
# 		for operand in operands[1:]:

# 		return result
# 	return wrapper

operands = operand_generator(40)
next(operands)

# - [ ] add test_generator

# def test_generator(count, builtin_dict):
#     # convert once to a list of tuple; allows subscripting
#     builtinlist = list(builtins.items())
#     builtincount = len(builtinlist)
#     for n in range(count):
#         # yield builtinlist[int(random.random() * builtincount)]
#         yield Test(gen_input, expected_result)


# test_suite = test_generator(10, builtins)

# --------------------------------------------- handwritten tests definition ---
Test = namedtuple("Test", "input result")

token_specs = [
    ("symbol", r"[a-zA-Z_+\-*%^\/\\=<>!&]+(?=[\s{}()])"),
    ("number", r"[-]?[0-9]+[.]?[0-9]*([eE][-+]?[0-9]+)?"),
]
regexp = re.compile("|".join("(?P<%s>%s)" % pair for pair in token_specs))


def pretty_replace(matches):
    if matches.group("number") is None:
        return f"{fg_cyan}{matches.group('symbol')}{reset}"
    else:
        return f"{fg_yellow}{matches.group('number')}{reset}"


def prettify(line):
    """Adorn a test expected result definition line with ansi esc code
    If lispy cosmetics change, this need to be changed accordingly
    """
    if "Error : " in line:
        line = line.replace(
            "Error : ", f"{fg_red}{reverse}Error : {reset}{fg_magenta}", 1
        )
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
# - [ ] log context and failures to file
#
# - [x] work from known/last good build output to generate tests
# 		- [x] support Test(input, n lines result)
#
# - [x] define separate decoration rule once
# - [ ] control the flow with func dict as with builtin_operators
# - [ ] key into flow dict with dict.get(key, defaultvalue)
def main() -> None:
    """
	docopt based CLI
	 
	See : http://docopt.org/
	"""
    arguments = docopt(__doc__, version="0.2")

    if arguments["--stress"] is None:
        arguments["--stress"] = 10
    if arguments["--delay"] is None:
        arguments["--delay"] = 0.05
    if arguments["--log"] is None:
        arguments["--log"] = "test_report.yaml"
    if arguments["--test"] is None:
        arguments["--test"] = "tests/test.lisp"

    # Get your bearings, setup, spawn process
    currentDirectory = os.getcwd()
    print(currentDirectory)

    # stressiteration = int(arguments["--stress"])
    verbose = arguments["-v"] | arguments["--verbose"]

    prompt = f"{bg_green}{fg_green}lispy {fg_black}> {reset}"

    testee = pexpect.spawn(arguments["<binary>"], encoding="utf-8")

    testee.delaybeforesend = float(arguments["--delay"])

    if arguments["-o"] | arguments["--output"]:
        testee.logfile_read = sys.stdout

    logfile_path = arguments["--log"]

    passed = 0
    failed = 0

    # Testing !
    with open(logfile_path, "w") as logfile:
        timestamp = datetime.datetime.now().isoformat()

        # try:
        tests = read_tests(arguments["--test"])
        # except:

        # use a generous timeout length for valgrind and/or program startup
        # testee.timeout = 5
        testee.expect_exact(prompt, timeout=5)

        # use a short timeout length to avoid lingering on failures
        testee.timeout = 0.5

        # Loop over hand written tests
        for test in tests:
            try:
                if verbose:
                    print(f"\x1b[30;43m ? \t\x1b[0;33m{test.input}\x1b[0m")
                testee.sendline(test.input)
                for line in test.result:
                    testee.expect_exact(f"{line}")
            except pexpect.exceptions.TIMEOUT:
                if verbose:
                    print("\x1b[31;7m ! \t            failed.\x1b[0m")
                else:
                    print("\x1b[31;7m ! ", end="", flush=True)
                logfile.write(
                    "\n--- # ----------------------------------------------------------------- TEST ---\n"
                    f"input           : {test.input}\n"
                    f"expected_result : {test.result}\n\n"
                    f"dump: {str(testee)}"
                    "\n... # --------------------------------------------------------------- FAILED ---\n"
                )
                failed += 1
            else:
                testee.expect_exact(prompt)
                if verbose:
                    print("\x1b[36;7m + \t            passed.\x1b[0m")
                else:
                    print("\x1b[36;7m + ", end="", flush=True)
                passed += 1

        # stretching lispenv a bit
        # for x in range(stressiteration):
        #     for y in range(10):
        #         a = random.randint(x, 256)
        #         b = random.randint(y, 256)
        #         symbol = [random_symbol(x + 1), random_symbol(y + 2)]
        #         operator = random.randint(0, 3)
        #         result = operators[operator](a, b)
        #         try:
        #             if verbose:
        #                 print(
        #                     f"\x1b[30;43m ? \t\x1b[0;33m{builtins[operator]}"
        #                     f" {a} {b} = {result}\x1b[0m"
        #                 )
        #             testee.sendline(f"def {{{symbol[0]} {symbol[1]}}} {a} {b}")
        #             testee.sendline(f"{builtins[operator]} {symbol[0]} {symbol[1]}")
        #             testee.expect(f"{result}")
        #         except pexpect.exceptions.TIMEOUT:
        #             if verbose:
        #                 print("\x1b[31;7m ! \t            failed.\x1b[0m")
        #             else:
        #                 print("\x1b[31;7m ! ", end="", flush=True)
        #             logfile.write(
        #                 "\n--- # ----------------------------------------------------------------- TEST ---\n"
        #                 f"input           : {builtins[operator]} {symbol[0]} {symbol[1]}\n"
        #                 f"expected_result : {result}\n\n"
        #                 f"dump: {str(testee)}"
        #                 "\n... # --------------------------------------------------------------- FAILED ---\n"
        #             )
        #             failed += 1
        #         else:
        #             testee.expect(prompt)
        #             if verbose:
        #                 print("\x1b[36;7m + \t            passed.\x1b[0m")
        #             else:
        #                 print("\x1b[36;7m + ", end="", flush=True)
        #             passed += 1

        # exit from inside spawned process
        testee.sendline("exit")
        testee.expect("exit")

        # valgrind
        # todo
        # - [ ] make Valgrind optional
        # - [x] add Valgrinds heap and error summary to test_report.yaml
        logfile.write("---\nvalgrind : '\n")
        testee.logfile_read = logfile
        testee.expect("ERROR SUMMARY:")
        logfile.write("'\n")

        # final tally
        print(
            f"\n\x1b[92;7m + \t            PASSED : {passed}\x1b[0m"
            f"\n\x1b[91;7m ! \t            FAILED : {failed}\x1b[0m"
        )
        logfile.write(
            f"start_date : {timestamp}\n"
            f"end_date   : {datetime.datetime.now().isoformat()}\n"
            f"options    : {str(arguments)}\n"
            f"passed     : {passed}\n"
            f"failed     : {failed}\n"
            "..."
        )

        print(f"log available : {logfile_path}")

    testee.close()


if __name__ == "__main__":
    main()

