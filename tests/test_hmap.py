"""\x1B[32;7m
 ┌────────────────────────────────────────────────────────────────────────────┐ 
 │  test_hmap.py                                                              │ 
 └────────────────────────────────────────────────────────────────────────────┘ 
\x1B[0;32;49m
███ Usage:
	test_hmap.py  <binary> [-k | --kill]
	test_hmap.py  -h | --help
	test_hmap.py  --version    

███ Options:
	-h, --help     Show this screen.
	--version      Show version.
	-k, --kill     Kill spawned process when done instead of yielding control back.
\x1B[0m
Run given hmap_test binary and feed it a hardcoded series of inputs.
────────────────────────────────────────────────────────────────────────────────
"""
from docopt import docopt
import pexpect
import os
import sys

inputs = [
    "256",
    "fill3Mf",
    "rm",
    "fill",
    # "fill1M",
    # "fill3Mf",
    # "findin",
    # "findrandin",
    # "findrand",
    # "findallsilent",
    # "stats",
    # "rm",
    # "fill3Mf",
    # "findin",
    # "findrandin",
    # "findrand",
    # "findallsilent",
    "stats",
]
# inputs = [
#     "10",
#     "fill10",
#     "fill10",
# ]


def main() -> None:
    """
	docopt based CLI.
	 
	See : http://docopt.org/
	"""
    arguments = docopt(__doc__, version="0.1")
    print(arguments)
    kill = arguments["-k"] | arguments["--kill"]

    # Get your bearings.
    currentDirectory = os.getcwd()
    print(currentDirectory)

    # fputs(FG_GREEN BG_BRIGHT_BLACK " > " RESET, stdout);
    prompt = f"\x1b[32m\x1b[100m > \x1b[0m"

    # Spawn process
    testee = pexpect.spawn(arguments["<binary>"], encoding="utf-8")
    testee.delaybeforesend = 0.05
    testee.logfile = sys.stdout

    # Use a generous timeout length for valgrind and/or program startup.
    testee.expect_exact(prompt, timeout=5)
    # testee.timeout = 0.5

    for input in inputs:
        testee.sendline(input)
        testee.expect_exact(prompt)

    if kill:
        # Exit from inside spawned process.
        testee.sendline("exit")

        # Close connection with spawned process.
        testee.close()
    else:
        # Give back control of spawned process to the user.
        testee.logfile = None
        testee.interact()


if __name__ == "__main__":
    main()
