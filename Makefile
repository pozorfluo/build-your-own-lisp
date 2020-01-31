HEADERDIR = ./include
SOURCEDIR = ./src
BINARYDIR = ./bin
TESTSDIR = ./tests

CFLAGS = -Wall -Wextra -Werror -pedantic -g -fsanitize=address
OFLAG = -Og
DEBUGFLAGS = -DVALGRIND
LIBFLAGS = -lm

# := is important to avoid running find every time SRC is inspected
SRC := $(shell find $(SOURCEDIR) -name '*.c')
# OBJ = $(SRC:.cc=.o)
EXEC = lispy

$(EXEC): 
	$(CC) $(CFLAGS) $(DEBUGFLAGS) $(OFLAG) $(LIBFLAGS) -o $(BINARYDIR)/$@ $(SRC)

clean:
	rm -rf $(BINARYDIR)/$(EXEC)

test:
	expect $(TESTSDIR)/test.exp $(BINARYDIR)/$(EXEC)