HEADERDIR = ./include
SOURCEDIR = ./src
BINARYDIR = ./bin
TESTSDIR = ./tests

CFLAGS = -Wall -Wextra -Werror -pedantic -g #-fsanitize=address -static-libasan
OFLAG = -Og
DEBUGFLAGS = -DVALGRIND #-DDEBUG_MALLOC
LIBFLAGS = -lm

# := is important to avoid running find every time SRC is inspected
SRC := $(shell find $(SOURCEDIR) -name '*.c')
# OBJ = $(SRC:.cc=.o)
EXEC = lispy
VALGRIND = "valgrind --track-origins=yes --leak-check=full --show-reachable=yes $(BINARYDIR)/$(EXEC)"

$(EXEC): 
	$(CC) $(CFLAGS) $(DEBUGFLAGS) $(OFLAG) $(LIBFLAGS) -o $(BINARYDIR)/$@ $(SRC)

clean:
	rm -rf $(BINARYDIR)/$(EXEC)

test:
	# Raise --delay=<s> if there are false positive :
	# It looks like it slows down after many malloc/free
	# and pexpect sends the next line 'too fast'.
	# The test then fails and it hangs for pexpect timeout duration.
	python $(TESTSDIR)/test.py $(VALGRIND) --delay=0.05 --stress=10 -v -o
	