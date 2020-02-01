#!/usr/bin/env python

from __future__ import absolute_import
from __future__ import print_function
from __future__ import unicode_literals

import pexpect
import re

import sys
import os
import random
import string
from collections import namedtuple


reset = "\x1b\[0m"
fg_black = "\x1b\[30m"
fg_green = "\x1b\[32m"
fg_cyan = "\x1b\[36m"
bg_green = "\x1b\[102m"

Test = namedtuple('Test', 'input result')

tests = [
	Test("+ 5 5",
		"10.00000"),
	Test("tail {qsd kkl aio}",
		f"{{{fg_cyan}kkl{reset} {fg_cyan}aio{reset} }}")
]

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

def random_symbol(length):
	return ''.join(random.choice(string.ascii_letters) for i in range(length))

def main():
	'''
		Loop over tests
	'''
	currentDirectory = os.getcwd()
	print(currentDirectory)

	# f"\x1B[{foreground};{background};m{text}"
	prompt = f"{bg_green}{fg_green}lispy {fg_black}> {reset}"
	
	testee = pexpect.spawn("bin/lispy", encoding='utf-8')
	testee.timeout = 2
	# file_log = open('mylog.txt','wb')
	# testee.logfile = sys.stdout
	testee.logfile_read = sys.stdout
	# testee.logfile_send = sys.stdout
	# testee.logfile = file_log
	# testee.delaybeforesend = 0.05

	testee.expect(prompt)	


	# print(testee)

	for test in tests:
		testee.sendline(test.input)
		testee.expect(test.result)
		testee.expect(prompt)
		# print(f"\n-------? {test.input} -> {test.result}")

	operators = [op_add, op_sub, op_mul, op_mod]
	builtins = ["add", "sub", "mul", "mod"]

	for x in range(10):
		for y in range(10):
			a = random.randint(x, 256)
			b = random.randint(y, 256)
			symbol = [random_symbol(x+1), random_symbol(y+2)]
			operator = random.randint(0, 3)
			result = operators[operator](a, b)
			# print(f"\n-------? {builtins[operator]} {a} {b} = {result}")
			testee.sendline(f"def {{{symbol[0]} {symbol[1]}}} {a} {b}")
			testee.sendline(f"{builtins[operator]} {symbol[0]} {symbol[1]}")
			testee.expect(f"{result}")
			testee.expect(prompt)

	print(f"ORA ORA ORA !")
	testee.close()
	
if __name__ == '__main__':
    main()