03:44 $ bin/hashmap_bucket_aos
__AVX__ 1
PROBE_LENGTH 32
__WORDSIZE 64
RAND_MAX  2147483647
Table size is 2^n. Enter n ( default n = 8 ) ? 26
Enter desired load factor ? .95
load_factor = 0.950000
load_count  = 63753420
Filling hashmap with 63753420 entries
hmap->top : 0
Done !
hmap->top : 63753420
|><| 2.796133 s
 > fill
hmap->top : 63753420
|><| 8.741018 s
 > find
sum : 4999950000
sum : 0
sum : 3023093005563
TEST_COUNT : 100000
|><| 0.020759 s
 > findrand
sum : 3025999964070
TEST_COUNT : 100000
|><| 0.014403 s
 > exit


03:45 $ bin/hashmap_bucket_soa_noprobe
__AVX__ 1
PROBE_LENGTH 32
__WORDSIZE 64
RAND_MAX  2147483647
Table size is 2^n. Enter n ( default n = 8 ) ? 26
Enter desired load factor ? .95
load_factor = 0.950000
load_count  = 63753420
Filling hashmap with 63753420 entries
hmap->top : 0
Done !
hmap->top : 63753420
|><| 3.836521 s
 > fill
hmap->top : 63753420
|><| 7.729750 s
 > find
sum : 4999950000
sum : 0
sum : 3023093005563
TEST_COUNT : 100000
|><| 0.021165 s
 > findrand
sum : 3025999964070
TEST_COUNT : 100000
|><| 0.014796 s
 > exit

03:46 $ bin/hashmap
__WORDSIZE 64
RAND_MAX  2147483647
Table size is 2^n. Enter n ( default n = 8 ) ? 26
Enter desired load factor ? .95
load_factor = 0.950000
load_count  = 63753420
Filling hashmap with 63753420 entries
hmap->top : 0
Done !
hmap->top : 63753420
|><| 10.205715 s
 > fill
hmap->top : 63753420
|><| 14.437058 s
 > find
sum : 4999950000
sum : 0
sum : 3023116543483
TEST_COUNT : 100000
|><| 0.026840 s
 > findrand
sum : 3025959790157
TEST_COUNT : 100000
|><| 0.


03:47 $ bin/hashmap
__AVX__ 1
PROBE_LENGTH 32
__WORDSIZE 64
RAND_MAX  2147483647
Table size is 2^n. Enter n ( default n = 8 ) ? 26
Enter desired load factor ? .95
load_factor = 0.950000
load_count  = 63753420
Filling hashmap with 63753420 entries
hmap->top : 0
Done !
hmap->top : 63753420
|><| 11.076323 s
 > fill
hmap->top : 63753420
|><| 15.009190 s
 > find
sum : 4999950000
sum : 0
sum : 3023093005563
TEST_COUNT : 100000
|><| 0.026781 s
 > findrand
sum : 3025999964070
TEST_COUNT : 100000
|><| 0.020687 s
 > exit

03:54 $ bin/hashmap_notlf
__AVX__ 1
PROBE_LENGTH 32
__WORDSIZE 64
RAND_MAX  2147483647
Table size is 2^n. Enter n ( default n = 8 ) ? 26
Enter desired load factor ? .95
load_factor = 0.950000
load_count  = 63753420
Filling hashmap with 63753420 entries
hmap->top : 0
Done !
hmap->top : 63753420
|><| 10.410741 s
 > fill
hmap->top : 63753420
|><| 14.844543 s
 > find
sum : 4999950000
sum : 0
sum : 3023093005563
TEST_COUNT : 100000
|><| 0.024803 s
 > findrand
sum : 3025999964070
TEST_COUNT : 100000
|><| 0.019800 s
 > exit

03:56 $ bin/hashmap_notlf
__WORDSIZE 64
RAND_MAX  2147483647
Table size is 2^n. Enter n ( default n = 8 ) ? 26
Enter desired load factor ? .95
load_factor = 0.950000
load_count  = 63753420
Filling hashmap with 63753420 entries
hmap->top : 0
Done !
hmap->top : 63753420
|><| 10.782148 s
 > fill
hmap->top : 63753420
|><| 14.660831 s
 > find
sum : 4999950000
sum : 0
sum : 3023116543483
TEST_COUNT : 100000
|><| 0.027151 s
 > findrand
sum : 3025959790157
TEST_COUNT : 100000
|><| 0.019938 s
 > exit

03:58 $ bin/hashmap_notlf
__WORDSIZE 64
RAND_MAX  2147483647
Table size is 2^n. Enter n ( default n = 8 ) ? 26
Enter desired load factor ? .5
load_factor = 0.500000
load_count  = 33554432
Filling hashmap with 33554432 entries
hmap->top : 0
Done !
hmap->top : 33554432
|><| 5.672889 s
 > fill
hmap->top : 33554432
|><| 7.725984 s
 > find
sum : 4999950000
sum : 0
sum : 837218366826
TEST_COUNT : 100000
|><| 0.024224 s
 > findrand
sum : 838601591725
TEST_COUNT : 100000
|><| 0.014899 s
 > exit

 04:00 $ bin/hashmap_bucket_aos
__AVX__ 1
PROBE_LENGTH 32
__WORDSIZE 64
RAND_MAX  2147483647
Table size is 2^n. Enter n ( default n = 8 ) ? 26
Enter desired load factor ? .5
load_factor = 0.500000
load_count  = 33554432
Filling hashmap with 33554432 entries
hmap->top : 0
Done !
hmap->top : 33554432
|><| 1.330931 s
 > fill
hmap->top : 33554432
|><| 4.584308 s
 > find
sum : 4999950000
sum : 0
sum : 837206001754
TEST_COUNT : 100000
|><| 0.018433 s
 > findrand
sum : 838656289979
TEST_COUNT : 100000
|><| 0.013174 s
 > exit

04:02 $ bin/hashmap_bucket_soa_noprobe
__AVX__ 1
PROBE_LENGTH 32
__WORDSIZE 64
RAND_MAX  2147483647
Table size is 2^n. Enter n ( default n = 8 ) ? 26
Enter desired load factor ? .5
load_factor = 0.500000
load_count  = 33554432
Filling hashmap with 33554432 entries
hmap->top : 0
Done !
hmap->top : 33554432
|><| 2.087907 s
 > fill
hmap->top : 33554432
|><| 4.098571 s
 > find
sum : 4999950000
sum : 0
sum : 837206001754
TEST_COUNT : 100000
|><| 0.018096 s
 > findrand
sum : 838656289979
TEST_COUNT : 100000
|><| 0.014172 s
 > exit


04:12 $ bin/hashmap_bucket_aos
todo
__WORDSIZE 64
RAND_MAX  2147483647
Table size is 2^n. Enter n ( default n = 8 ) ? 26
Enter desired load factor ? .95
load_factor = 0.950000
load_count  = 63753420
Filling hashmap with 63753420 entries
hmap->top : 0
Done !
hmap->top : 63753420
|><| 2.809872 s
 > rm
hmap->top : 0
|><| 9.590436 s
 > fill
hmap->top : 63753420
|><| 2.485989 s
 > find
sum : 4999950000
sum : 0
sum : 3023116543483
TEST_COUNT : 100000
|><| 0.020729 s
 > findrand
sum : 3025959790157
TEST_COUNT : 100000
|><| 0.016192 s
 > exit

04:13 $ bin/hashmap_bucket_aos
__AVX__ 1
PROBE_LENGTH 32
__WORDSIZE 64
RAND_MAX  2147483647
Table size is 2^n. Enter n ( default n = 8 ) ? 26
Enter desired load factor ? .95
load_factor = 0.950000
load_count  = 63753420
Filling hashmap with 63753420 entries
hmap->top : 0
Done !
hmap->top : 63753420
|><| 2.659032 s
 > rm
hmap->top : 0
|><| 9.960299 s
 > find
sum : 0
sum : 0
sum : 0
TEST_COUNT : 100000
|><| 0.011202 s
 > findrand
sum : 0
TEST_COUNT : 100000
|><| 0.006711 s
 > fill
hmap->top : 63753420
|><| 2.302729 s
 > find
sum : 4999950000
sum : 0
sum : 3018078546553
TEST_COUNT : 100000
|><| 0.020677 s
 > findrand
sum : 3023981450019
TEST_COUNT : 100000
|><| 0.013892 s
 > exit

04:24 $ bin/hashmap_bucket_soa_noprobe
__AVX__ 1
PROBE_LENGTH 32
__WORDSIZE 64
RAND_MAX  2147483647
Table size is 2^n. Enter n ( default n = 8 ) ? 26
Enter desired load factor ? .95
load_factor = 0.950000
load_count  = 63753420
Filling hashmap with 63753420 entries
hmap->top : 0
Done !
hmap->top : 63753420
|><| 3.718017 s
 > rm
hmap->top : 0
|><| 12.646103 s
 > fill
hmap->top : 63753420
|><| 3.406086 s
 > find
sum : 4999950000
sum : 0
sum : 3023093005563
TEST_COUNT : 100000
|><| 0.019821 s
 > finrand
Error : could NOT process given seed : junk !
|><| 0.000029 s
 > findrand
sum : 3025999964070
TEST_COUNT : 100000
|><| 0.013252 s
 > exit

04:26 $ bin/hashmap_notlf
__WORDSIZE 64
RAND_MAX  2147483647
Table size is 2^n. Enter n ( default n = 8 ) ? 26
Enter desired load factor ? .95
load_factor = 0.950000
load_count  = 63753420
Filling hashmap with 63753420 entries
hmap->top : 0
Done !
hmap->top : 63753420
|><| 10.590874 s
 > rm
hmap->top : 0
|><| 24.339489 s
 > fill
hmap->top : 63753420
|><| 10.117825 s
 > find
sum : 4999950000
sum : 0
sum : 3023116543483
TEST_COUNT : 100000
|><| 0.027802 s
 > findrand
sum : 3025959790157
TEST_COUNT : 100000
|><| 0.023033 s
 > exit