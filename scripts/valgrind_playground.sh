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
valgrind --tool=cachegrind bin/baseline-array
valgrind --tool=cachegrind bin/hashmap
valgrind --tool=cachegrind bin/lispy

# annotate profiling infos
cg_annotate --auto=yes cachegrind.out.* > cachegrind.out.annotated


# merge profiles
cg_merge -o outputfile file1 file2 file3 ...

# diff profiles
cg_diff file1 file2

# run profile script
python ./scripts/profile_hmap.py "valgrind --tool=cachegrind bin/hashmap" -o
python ./scripts/profile_hmap.py "valgrind --tool=cachegrind bin/hashmap_bucket_aos" -o
python ./scripts/profile_hmap.py "valgrind --tool=cachegrind bin/hashmap_bucket_soa_noprobe" -o
python ./scripts/profile_hmap.py "valgrind --tool=cachegrind bin/baseline-array" -o



cg_annotate --auto=yes cachegrind.out.4758 src/hashmap.c > cachegrind.out.annotated
cg_annotate --auto=yes cachegrind.out.30923 > cachegrind.out.annotated
cg_diff cachegrind.out.30310 cachegrind.out.4758