#!/bin/bash

# make this script executable by owner
# chmod u+x ./test.sh

# pass a ton of input to tested program using Heredoc
valgrind --track-origins=yes --leak-check=full --show-reachable=yes ../bin/lisp\
<<LISPY
list
init
eval
join
cons
len
def
sub
mul
add
div
mod
pow
max
min
+
-
*
/
%
^
> 
<
exit
LISPY

# get expect package
# sudo apt-get install expect