# watch lispy command history
less +F -N lispy_history.txt

# run valgrind
valgrind bin/hashmap
valgrind bin/baseline
valgrind --track-origins=yes bin/lispy
valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all bin/lispy
valgrind --track-origins=yes --leak-check=full --show-reachable=yes bin/lispy

# run cachegrind
# see : https://valgrind.org/docs/manual/cg-manual.html
valgrind --tool=cachegrind bin/hashmap
valgrind --tool=cachegrind bin/baseline
valgrind --tool=cachegrind bin/lispy

# annotate profiling infos
cg_annotate --auto=yes cachegrind.out.* > cachegrind.out.annotated

# merge profiles
cg_merge -o outputfile file1 file2 file3 ...

# diff profiles
cg_diff file1 file2
