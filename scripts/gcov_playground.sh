# add build options
# "-fopt-info-vec-optimized",
# "-fprofile-arcs",
# // "-fprofile-abs-path",
# // "-ftest-coverage",
# "-p",
# "--coverage",
# // "-fprofile-dir=/home/spagbol/_source/C/build-your-own-lisp/src/",
# // "-fprofile-generate=/home/spagbol/_source/C/build-your-own-lisp/src/",

# annotate
gcov -b -u -q -a -f -k -t src/hmap.c