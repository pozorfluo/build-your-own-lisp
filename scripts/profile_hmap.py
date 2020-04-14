#!/usr/bin/env python
"""\x1B[32;7m
┌──────────────────────────────────────────────────────────────────────────────┐
│   profile_hmap.py                                                            │
└──────────────────────────────────────────────────────────────────────────────┘
\x1B[0;32;49m
███ Usage:
	profile_hmap.py  <binary> [--log=<file>]
					   [--delay=<s>]  [-v | --verbose] [-o | --output]
	profile_hmap.py  -h | --help
	profile_hmap.py  --version

███ Options:
	-h, --help     Show this screen.
	--version      Show version.
	--log=<file>   Path to log file [default: profile_report.tsv].
	--delay=<s>    Delay before sending commands [default: 0.05 seconds].
	-v, --verbose  Pretty prints test results in terminal.
	-o, --output   Show spawned process output in terminal.
\x1B[0m 
Launch, collect, arrange hmap cachegrind test results in a .tsv log file.

\x1B[90m 
────────────────────────────────────────────────────────────────────────
\x1B[0m
"""
from docopt import docopt
import pexpect

import sys
import os

import random
import string
import datetime

import itertools

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

fg_bright_black = "\x1b[90m"
fg_bright_red = "\x1b[91m"
fg_bright_green = "\x1b[92m"
fg_bright_yellow = "\x1b[93m"
fg_bright_blue = "\x1b[94m"
fg_bright_magenta = "\x1b[95m"
fg_bright_cyan = "\x1b[96m"
fg_bright_white = "\x1b[97m"

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

# --------------------------------------------- handwritten tests definition ---

tests = {
    "n": range(20),
    "load_factor": [0.1, 0.25, 0.5, 0.65, 0.75, 0.85, 0.90, 0.95, 0.98, 1],
}

# --------------------------------------------------------------------- main ---
def main() -> None:
    """
	docopt based CLI
	 
	See : http://docopt.org/
	"""
    arguments = docopt(__doc__, version="0.1")

    if arguments["--delay"] is None:
        arguments["--delay"] = 0.05
    if arguments["--log"] is None:
        arguments["--log"] = "profile_report.tsv"

    # Get your bearings, setup
    currentDirectory = os.getcwd()
    print(currentDirectory)
    # verbose = arguments["-v"] | arguments["--verbose"]
    logfile_path = arguments["--log"]

    # ---------------------------------------------------------- prompts
    prompt = f"{bg_green} > {reset}"
    prompt_size = (
        f"{fg_bright_blue}{reverse}"
        f"Table size is 2^n. Enter n ( default n = 8 ) ? {reset}"
    )
    prompt_load = f"{fg_blue}{reverse}Enter desired load factor ? {reset}"

    # -------------------------------------------------- regexp butchery
    # exactly that format or fail
    cachegrind_re = re.compile(
        # r"\n==\d+== \n"
        r"==\d+== I   refs:\s+(?P<i_refs>[0-9,]+).*\n"
        r"==\d+== I1  misses:\s+(?P<i_misses>[0-9,]+).*\n"
        r"==\d+== LLi misses:\s+(?P<lli_misses>[0-9,]+).*\n"
        r"==\d+== I1  miss rate:\s+(?P<i_missrate>[0-9,.%]+).*\n"
        r"==\d+== LLi miss rate:\s+(?P<lli_missrate>[0-9,.%]+).*\n"
        r"==\d+==.*\n"
        r"==\d+== D   refs:\s+(?P<d_refs>[0-9,]+)\s+\(\s*(?P<d_refs_rd>[0-9,]+) rd\s+\+\s+(?P<d_refs_wr>[0-9,]+) wr\).*\n"
        r"==\d+== D1  misses:\s+(?P<d_misses>[0-9,]+)\s+\(\s*(?P<d_misses_rd>[0-9,]+) rd\s+\+\s+(?P<d_misses_wr>[0-9,]+) wr\).*\n"
        r"==\d+== LLd misses:\s+(?P<lld_misses>[0-9,]+)\s+\(\s*(?P<lld_misses_rd>[0-9,]+) rd\s+\+\s+(?P<lld_misses_wr>[0-9,]+) wr\).*\n"
        r"==\d+== D1  miss rate:\s+(?P<d_missrate>[0-9,.%]+)\s+\(\s*(?P<d_missrate_rd>[0-9,.%]+)\s+\+\s+(?P<d_missrate_wr>[0-9,.%]+)\s*\).*\n"
        r"==\d+== LLd miss rate:\s+(?P<lld_missrate>[0-9,.%]+)\s+\(\s*(?P<lld_missrate_rd>[0-9,.%]+)\s+\+\s+(?P<lld_missrate_wr>[0-9,.%]+)\s*\).*\n"
        r"==\d+==.*\n"
        r"==\d+== LL refs:\s+(?P<ll_refs>[0-9,]+)\s+\(\s*(?P<ll_refs_rd>[0-9,]+) rd\s+\+\s+(?P<ll_refs_wr>[0-9,]+) wr\).*\n"
        r"==\d+== LL misses:\s+(?P<ll_misses>[0-9,]+)\s+\(\s*(?P<ll_misses_rd>[0-9,]+) rd\s+\+\s+(?P<ll_misses_wr>[0-9,]+) wr\).*\n"
        r"==\d+== LL miss rate:\s+(?P<ll_missrate>[0-9,.%]+)\s+\(\s*(?P<ll_missrate_rd>[0-9,.%]+)\s+\+\s+(?P<ll_missrate_wr>[0-9,.%]+)\s*\).*\n",
        re.MULTILINE,
    )

    # test_str = (
    #     "==16142== \n"
    #     "==16142== I   refs:      238,103\n"
    #     "==16142== I1  misses:      1,432\n"
    #     "==16142== LLi misses:      1,363\n"
    #     "==16142== I1  miss rate:    0.60%\n"
    #     "==16142== LLi miss rate:    0.57%\n"
    #     "==16142== \n"
    #     "==16142== D   refs:       85,671  (62,254 rd   + 23,417 wr)\n"
    #     "==16142== D1  misses:      3,478  ( 2,732 rd   +    746 wr)\n"
    #     "==16142== LLd misses:      2,762  ( 2,102 rd   +    660 wr)\n"
    #     "==16142== D1  miss rate:     4.1% (   4.4%     +    3.2%  )\n"
    #     "==16142== LLd miss rate:     3.2% (   3.4%     +    2.8%  )\n"
    #     "==16142== \n"
    #     "==16142== LL refs:         4,910  ( 4,164 rd   +    746 wr)\n"
    #     "==16142== LL misses:       4,125  ( 3,465 rd   +    660 wr)\n"
    #     "==16142== LL miss rate:      1.3% (   1.2%     +    2.8%  )"
    # )

    # Testing !
    with open(logfile_path, "w") as logfile:
        # timestamp = datetime.datetime.now().isoformat()

        for n, load_factor in itertools.product(tests["n"], tests["load_factor"]):
            print(f"{n} : {load_factor}")
            # spawn process
            testee = pexpect.spawn(arguments["<binary>"], encoding="utf-8")
            testee.delaybeforesend = float(arguments["--delay"])

            if arguments["-o"] | arguments["--output"]:
                testee.logfile_read = sys.stdout
            # use a generous timeout length for valgrind and/or program startup
            # testee.timeout = 5
            testee.expect_exact(prompt_size, timeout=5)

            # use a short timeout length to avoid lingering on failures
            testee.timeout = 0.5

            # dummy test
            testee.sendline(str(n))
            testee.expect_exact(prompt_load)
            testee.sendline(str(load_factor))
            testee.expect_exact(prompt)

            # todo
            #   - [ ] Catch timer results
            #         |><| 0.409057 s


            # # Loop over hand written tests
            # # for test in tests:
            #     try:
            #         if verbose:
            #             print(f"\x1b[30;43m ? \t\x1b[0;33m{test.input}\x1b[0m")
            #         testee.sendline(test.input)
            #         for line in test.result:
            #             testee.expect_exact(f"{line}")
            #     except pexpect.exceptions.TIMEOUT:
            #         if verbose:
            #             print("\x1b[31;7m ! \t            failed.\x1b[0m")
            #         else:
            #             print("\x1b[31;7m ! ", end="", flush=True)
            #         logfile.write(
            #             "\n--- # ----------------------------------------------------------------- TEST ---\n"
            #             f"input           : {test.input}\n"
            #             f"expected_result : {test.result}\n\n"
            #             f"dump: {str(testee)}"
            #             "\n... # --------------------------------------------------------------- FAILED ---\n"
            #         )
            #         failed += 1
            #     else:
            #         testee.expect_exact(prompt)
            #         if verbose:
            #             print("\x1b[36;7m + \t            passed.\x1b[0m")
            #         else:
            #             print("\x1b[36;7m + ", end="", flush=True)
            #         passed += 1

            # exit from inside spawned process
            testee.sendline("exit")
            testee.expect("exit")

            # ------------------------------------------------------- tsv log
            cachegrind_output = testee.read()
            # print(cachegrind_output)
            # logfile.write(cachegrind_output)
            matches = cachegrind_re.search(cachegrind_output).groupdict()

            # todo
            #   - [ ] Log n, load_factor, load_count, timer results
            
            # log column name
            # for group_name in matches.keys():
            #     logfile.write(f"{group_name}\t")
            # logfile.write("\n")

            # log results
            for group_match in matches.values():
                logfile.write(f"{group_match.replace(',','')}\t")
            logfile.write("\n")
            # ----------------------------------------------- yaml style log
            # testee.logfile_read = logfile
            # testee.expect(pexpect.EOF)
            # logfile.write("---\ncachegrind : '")
            # for group_name, group_match in matches.groupdict().items():
            #     logfile.write(f"{group_name} : {group_match.replace(',','')}\n")
            # logfile.write(
            #     f"start_date : {timestamp}\n"
            #     f"end_date   : {datetime.datetime.now().isoformat()}\n"
            #     f"options    : {str(arguments)}\n"
            #     f"passed     : {passed}\n"
            #     f"failed     : {failed}\n"
            #     "..."
            # )
            # print(timestamp)
            testee.close()

    print(f"log available : {logfile_path}")


if __name__ == "__main__":
    main()
