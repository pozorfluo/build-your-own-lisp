#!/usr/bin/env python
u"""\x1B[32;7m
┌──────────────────────────────────────────────────────────────────────────────┐
│   tests.py                                                                   │
└──────────────────────────────────────────────────────────────────────────────┘
\x1B[0;32;49m
███ Usage:
	tests.py  <binary> [--stress=<n>] [--delay=<s>] [--log=<file>] [-v | --verbose] [-o | --output]
	tests.py  -h | --help
	tests.py  --version

███ Options:
	-h, --help     Show this screen.
	--version      Show version.
	--stress=<n>   Number of iterations for Lispenv test [default: 10].
	--delay=<s>    Delay before sending commands [default: 0.01 seconds].
	--log=<file>   Redirect output to a custom log file [default: test_report.yaml]
	-v, --verbose  Pretty prints test results in terminal
	-o, --output   Show spawned process output in terminal

\x1B[90m
	todo
	- [ ] peak at valgrind output
 
\x1B[0m
"""
# from __future__ import absolute_import
# from __future__ import print_function
# from __future__ import unicode_literals

from docopt import docopt
import pexpect
import re

from sys import stdout 
import os
import random
import string
# import time
import datetime
from collections import namedtuple

# Brackets are escaped to be used with .expect(regexp)
reset = "\x1b\[0m"
fg_black = "\x1b\[30m"
fg_green = "\x1b\[32m"
fg_yellow = "\x1b\[33m"
fg_cyan = "\x1b\[36m"
bg_green = "\x1b\[102m"

Test = namedtuple('Test', 'input result')

tests = [
	Test("+ 5 5",
		"10.00000"),
	Test("+ 1 (* 7 5) 3",
		"39.000000"),
	Test("(- 100)",
		"-100.000000"),
	Test("/",
		"<div>"),
	Test("(/ ())",
		"Error :"),
	Test("",
		"()"),
	Test("list 1 2 3 4",
		f"{{{fg_yellow}1.000000{reset} {fg_yellow}2.000000{reset} {fg_yellow}3.000000{reset} {fg_yellow}4.000000{reset} }}"),
	# Test("{head (list 1 2 3 4)}",
	# 	f"{head (list 1.000000 2.000000 3.000000 4.000000 ) }"),
	Test("eval {head (list 1 2 3 4)}",
		f"{{{fg_yellow}1.000000{reset} }}"),
	Test("tail {tail tail tail}",
		f"{{{fg_cyan}tail{reset} {fg_cyan}tail{reset} }}"),
	Test("eval (tail {tail tail {5 6 7}})",
		f"{{{fg_yellow}6.000000{reset} {fg_yellow}7.000000{reset} }}"),
	Test("eval (head {(+ 1 2) (+ 10 20)})",
		"3.000000"),
	Test("tail {qsd kkl aio}",
		f"{{{fg_cyan}kkl{reset} {fg_cyan}aio{reset} }}")
]

# def get_speak_func(text, volume):
#     def whisper():
#         return text.lower() + '...'
#     def yell():
#         return text.upper() + '!'
#     if volume > 0.5:
#         return yell
#     else:
#         return whisper

def op_add(a, b):
	return a + b

def op_sub(a, b):
	return a - b

def op_mul(a, b):
	return a * b

def op_mod(a, b):
	if b == 0:
		return "-nan"
	else:
		return a % b

# op_add = lambda a, b: a + b	
# op_sub = lambda a, b: a - b	

def random_symbol(length):
	return ''.join(random.choice(string.ascii_letters) for i in range(length))

def main() -> None:
	"""
    docopt based CLI
     
    See : http://docopt.org/
    """
	arguments = docopt(__doc__, version="0.1")

	if arguments['--stress'] is None:
		arguments['--stress'] = 10
	if arguments['--delay'] is None:
		arguments['--delay'] = 0.05
	if arguments['--log'] is None:
		arguments['--log'] = 'test_report.yaml'


	
	currentDirectory = os.getcwd()
	print(currentDirectory)

	# spawn process, setup
	stressiteration = int(arguments['--stress'])
	verbose = arguments['-v'] | arguments['--verbose']
	
	prompt = f"{bg_green}{fg_green}lispy {fg_black}> {reset}"	
	
	testee = pexpect.spawn(arguments['<binary>'], encoding='utf-8')
	testee.timeout = 1
	testee.delaybeforesend = float(arguments['--delay'])

	if (arguments['-o'] | arguments['--output']):
		testee.logfile_read = stdout
	# testee.logfile = logfile


	# testee.logfile = sys.stdout
	# testee.logfile_send = sys.stdout
	passed = 0
	failed = 0

# 2020-01-30 17:32:55
	with open(arguments['--log'], 'w') as logfile:
		# timestamp = time.strftime("%Y-%m-%d %H:%M:%S\n", time.gmtime())
		timestamp = datetime.datetime.now().isoformat()
		# print(timestamp)
		# print(str(arguments))
		testee.expect(prompt)	
		# Loop over tests
		for test in tests:
			try:
				if verbose:
					print(f"\x1b[30;43m ? \t\x1b[0;33m{test.input}\x1b[0m")
				testee.sendline(test.input)
				testee.expect(f"{test.result}")
			except pexpect.exceptions.TIMEOUT:
				if verbose:
					print("\x1b[31;7m ! \t            failed.\x1b[0m")
				else:
					print("\x1b[31;7m ! ", end = '', flush=True)
					# stdout.write("\x1b[31;7m ! ")
				logfile.write('\n--- # ----------------------------------------------------------------- TEST ---\n')
				logfile.write(f"input           : {test.input}\n")
				logfile.write(f"expected_result : {test.result}\n\n")
				logfile.write(f"dump: {str(testee)}")
				logfile.write('\n... # --------------------------------------------------------------- FAILED ---\n')
				failed+=1
			else:
				testee.expect(prompt)
				if verbose:
					print("\x1b[36;7m + \t            passed.\x1b[0m")
				else:
					print("\x1b[36;7m + ", end ='', flush=True)
				passed+=1

		# stretching lispenv a bit
		operators = [op_add, op_sub, op_mul, op_mod]
		builtins = ["add", "sub", "mul", "mod"]

		for x in range(stressiteration):
			for y in range(10):
				a = random.randint(x, 256)
				b = random.randint(y, 256)
				symbol = [random_symbol(x+1), random_symbol(y+2)]
				operator = random.randint(0, 3)
				result = operators[operator](a, b)
				try:
					if verbose:
						print(f"\x1b[30;43m ? \t\x1b[0;33m{builtins[operator]} {a} {b} = {result}\x1b[0m")
					testee.sendline(f"def {{{symbol[0]} {symbol[1]}}} {a} {b}")
					testee.sendline(f"{builtins[operator]} {symbol[0]} {symbol[1]}")
					testee.expect(f"{result}")
				except pexpect.exceptions.TIMEOUT:
					if verbose:
						print("\x1b[31;7m ! \t            failed.\x1b[0m")
					else:
						print("\x1b[31;7m ! ", end = '', flush=True)
					logfile.write('\n--- # ----------------------------------------------------------------- TEST ---\n')
					logfile.write(f"input           : {builtins[operator]} {symbol[0]} {symbol[1]}\n")
					logfile.write(f"expected_result : {result}\n\n")
					logfile.write(f"dump: {str(testee)}")
					logfile.write('\n... # --------------------------------------------------------------- FAILED ---\n')
					failed+=1
				else:
					testee.expect(prompt)
					if verbose:
						print("\x1b[36;7m + \t            passed.\x1b[0m")
					else:
						print("\x1b[36;7m + ", end ='', flush=True)
					passed+=1

		# exit from inside spawned process
		testee.sendline("exit")
		# todo
		#   - [ ] make Valgrind optional and handle its ouput
		testee.expect("ERROR SUMMARY:") 
		# final tally
		print(f"\n\x1b[92;7m + \t            PASSED : {passed}\x1b[0m")
		print(f"\x1b[91;7m ! \t            FAILED : {failed}\x1b[0m")
		logfile.write(f"\n---\n")
		logfile.write(f"start_date : {timestamp}\n")
		logfile.write(f"end_date   : {datetime.datetime.now().isoformat()}\n")
		logfile.write(f"options    : {str(arguments)}\n")
		logfile.write(f"passed     : {passed}\n")
		logfile.write(f"failed     : {failed}\n")
		logfile.write(f"...")

	testee.close()
	
if __name__ == '__main__':
    main()