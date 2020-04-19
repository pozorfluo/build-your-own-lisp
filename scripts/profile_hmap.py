#!/usr/bin/env python
"""\x1B[32;7m
┌──────────────────────────────────────────────────────────────────────────────┐
│   profile_hmap.py                                                            │
└──────────────────────────────────────────────────────────────────────────────┘
\x1B[0;32;49m
███ Usage:
	profile_hmap.py  <binary> [--log=<file>] [--delay=<s>] [-o | --output]
                              [--separator=<char>]
	profile_hmap.py  -h | --help
	profile_hmap.py  --version

███ Options:
	-h, --help           Show this screen.
	--version            Show version.
	--log=<file>         Path to log file [default: profile_report.tsv].
	--delay=<s>          Delay before sending commands [default: 0.05 seconds].
    --separator=<char>   Separator to use between values [default: \t]
	-o, --output         Show spawned process output in terminal.

\x1B[0m
Launch, collect, arrange hmap cachegrind test results in a .tsv log file.
\x1B[0;32;49m
────────────────────────────────────────────────────────────────────────────────
\x1B[0m
"""
from docopt import docopt
import pexpect

import sys
import os

import random
import string

# import datetime

import itertools

# from collections import namedtuple
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

# ----------------------------------------- Generated tests helper functions ---

# --------------------------------------------- handwritten tests definition ---
tests = {
    "n": range(25),
    # "load_factor": [0.1, 0.25, 0.5, 0.65, 0.75, 0.85, 0.90, 0.95, 0.98, 1],
    "load_factor": [0.1, 0.5, 0.95, 1],
    "commands": ["rm", "fill", "sum", "find", "findrand"],
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
    if arguments["--separator"] is None:
        arguments["--separator"] = "\t"

    # Get your bearings, setup
    currentDirectory = os.getcwd()
    print(currentDirectory)

    logfile_path = arguments["--log"]
    separator = arguments["--separator"]
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

    timer_re = re.compile(r"\|\>\<\| ([0-9.]+) s")
    count_re = re.compile(r"load_count  = ([0-9.]+)")

    # --------------------------------------------------------- test jobs
    with open(logfile_path, "w") as logfile:
        # log column name
        logfile.write(
            f"n{separator}"
            f"load_factor{separator}"
            f"entry_count{separator}"
            f"i_refs{separator}"
            f"i_misses{separator}"
            f"lli_misses{separator}"
            f"i_missrate{separator}"
            f"lli_missrate{separator}"
            f"d_refs{separator}"
            f"d_refs_rd{separator}"
            f"d_refs_wr{separator}"
            f"d_misses{separator}"
            f"d_misses_rd{separator}"
            f"d_misses_wr{separator}"
            f"lld_misses{separator}"
            f"lld_misses_rd{separator}"
            f"lld_misses_wr{separator}"
            f"d_missrate{separator}"
            f"d_missrate_rd{separator}"
            f"d_missrate_wr{separator}"
            f"lld_missrate{separator}"
            f"lld_missrate_rd{separator}"
            f"lld_missrate_wr{separator}"
            f"ll_refs{separator}"
            f"ll_refs_rd{separator}"
            f"ll_refs_wr{separator}"
            f"ll_misses{separator}"
            f"ll_misses_rd{separator}"
            f"ll_misses_wr{separator}"
            f"ll_missrate{separator}"
            f"ll_missrate_rd{separator}"
            f"ll_missrate_wr{separator}"
            f"initial_fill{separator}"
        )
        for command in tests["commands"]:
            logfile.write(f"{command}{separator}")

        logfile.write("\n")

        # loop through jobs
        for n, load_factor in itertools.product(tests["n"], tests["load_factor"]):
            print(f"job n = {n}; load_factor = {load_factor}")
            # spawn process
            testee = pexpect.spawn(arguments["<binary>"], encoding="utf-8")
            testee.delaybeforesend = float(arguments["--delay"])

            if arguments["-o"] | arguments["--output"]:
                testee.logfile_read = sys.stdout
            # use a generous timeout length for valgrind and/or program startup
            testee.expect_exact(prompt_size, timeout=5)

            # use a generous timeout, it takes a while past n > 24
            testee.timeout = 60

            timer_results = []

            # send job setup
            testee.sendline(str(n))
            testee.expect_exact(prompt_load)
            testee.sendline(str(load_factor))

            # catch load_count
            testee.expect(count_re)
            load_count = testee.match.group(1)
            # catch initial_fill timer result
            testee.expect(timer_re)
            timer_results.append(testee.match.group(1))

            # loop through job commands
            for command in tests["commands"]:
                testee.expect_exact(prompt)
                testee.sendline(command)
                # catch command timer result
                testee.expect(timer_re)
                timer_results.append(testee.match.group(1))

            # exit from inside spawned process
            testee.sendline("exit")
            testee.expect("exit")

            # ------------------------------------------------------- tsv log
            cachegrind_output = testee.read()
            # print(cachegrind_output)
            # logfile.write(cachegrind_output)
            matches = cachegrind_re.search(cachegrind_output).groupdict()

            # log job setup
            logfile.write(
                f"{n}{separator}" 
                f"{load_factor}{separator}"
                f"{load_count}{separator}"
            )

            # log results
            for group_match in matches.values():
                logfile.write(f"{group_match.replace(',','')}{separator}")

            # log timer results
            for result in timer_results:
                logfile.write(f"{result}{separator}")

            logfile.write("\n")

            testee.close()

    print(f"log available : {logfile_path}")


if __name__ == "__main__":
    main()
