HEADERDIR = ./include
SOURCEDIR = ./src
BINARYDIR = ./bin
TESTSDIR = ./tests

EXCLUDE = hashmap.c

IFLAGS = -iquote$(HEADERDIR)
CFLAGS = -Wall -Wextra -Werror -pedantic -g #-fsanitize=address -static-libasan
OFLAG = -Og
DEBUGFLAGS = -DVALGRIND #-DDEBUG_MALLOC
LIBFLAGS = -lm

# := is important to avoid running find every time SRC is inspected
SRC := $(shell find $(SOURCEDIR)/ ! -name $(EXCLUDE) -name '*.c')
# SRC := $(wildcard $(SOURCEDIR)*.c)
# SRC := $(filter-out $(SRC)/hashmap.c, $(SRC))
# SRC := $(subst $(EXCLUDE),,$(SRC))
# OBJ = $(SRC:.cc=.o)
EXEC = lispy
VALGRIND = "valgrind --track-origins=yes --leak-check=full --show-reachable=yes $(BINARYDIR)/$(EXEC)"

$(EXEC): 
	$(CC) $(IFLAGS) $(CFLAGS) $(DEBUGFLAGS) $(OFLAG) $(LIBFLAGS) -o $(BINARYDIR)/$@ $(SRC)

clean:
	rm -rf $(BINARYDIR)/$(EXEC)

test:
	# Raise --delay=<s> if there are false positive 
	python $(TESTSDIR)/test.py $(VALGRIND) -v -o
	