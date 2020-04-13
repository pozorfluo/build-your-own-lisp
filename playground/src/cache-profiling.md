# hash_tab, n = 12
```js
==26597== 
==26597== I   refs:      12,094,655
==26597== I1  misses:         1,244
==26597== LLi misses:         1,215
==26597== I1  miss rate:       0.01%
==26597== LLi miss rate:       0.01%
==26597== 
==26597== D   refs:       4,433,085  (2,948,471 rd   + 1,484,614 wr)
==26597== D1  misses:        26,868  (   18,569 rd   +     8,299 wr)
==26597== LLd misses:         8,299  (    2,123 rd   +     6,176 wr)
==26597== D1  miss rate:        0.6% (      0.6%     +       0.6%  )
==26597== LLd miss rate:        0.2% (      0.1%     +       0.4%  )
==26597== 
==26597== LL refs:           28,112  (   19,813 rd   +     8,299 wr)
==26597== LL misses:          9,514  (    3,338 rd   +     6,176 wr)
==26597== LL miss rate:         0.1% (      0.0%     +       0.4%  )
```

# reduce_fibo cache-profiling branch, n = 12
```js
==28796== 
==28796== I   refs:      584,375
==28796== I1  misses:      1,172
==28796== LLi misses:      1,154
==28796== I1  miss rate:    0.20%
==28796== LLi miss rate:    0.20%
==28796== 
==28796== D   refs:      265,263  (153,831 rd   + 111,432 wr)
==28796== D1  misses:      8,648  (  3,232 rd   +   5,416 wr)
==28796== LLd misses:      4,228  (  2,114 rd   +   2,114 wr)
==28796== D1  miss rate:     3.3% (    2.1%     +     4.9%  )
==28796== LLd miss rate:     1.6% (    1.4%     +     1.9%  )
==28796== 
==28796== LL refs:         9,820  (  4,404 rd   +   5,416 wr)
==28796== LL misses:       5,382  (  3,268 rd   +   2,114 wr)
==28796== LL miss rate:      0.6% (    0.4%     +     1.9%  )
```

# baseline array, n = 16
```js
Table size is 2^n. Enter n ( default n=8 )  ? 16
Filling hashmap with 64225 entries
Done !
 >  > rm
 > fill
 > sum
sum : 2062393200 | 2062393200
 > exit
==1018== 
==1018== I   refs:      937,576
==1018== I1  misses:      1,135
==1018== LLi misses:      1,115
==1018== I1  miss rate:    0.12%
==1018== LLi miss rate:    0.12%
==1018== 
==1018== D   refs:      357,027  (179,832 rd   + 177,195 wr)
==1018== D1  misses:     67,801  ( 18,936 rd   +  48,865 wr)
==1018== LLd misses:     18,711  (  2,099 rd   +  16,612 wr)
==1018== D1  miss rate:    19.0% (   10.5%     +    27.6%  )
==1018== LLd miss rate:     5.2% (    1.2%     +     9.4%  )
==1018== 
==1018== LL refs:        68,936  ( 20,071 rd   +  48,865 wr)
==1018== LL misses:      19,826  (  3,214 rd   +  16,612 wr)
==1018== LL miss rate:      1.5% (    0.3%     +     9.4%  )
```


# reduce_fibo cache-profiling branch, n = 16
```js
Table size is 2^n. Enter n ( default n=8 )  ? 16
Filling hashmap with 64225 entries
hmap->top : 0
Done !
hmap->top : 64225
 >  > rm
hmap->top : 0
 > fill
hmap->top : 64225
 > sums
sum_key             : 2062393200
sum_value           : 2062393200
 > exit
==1102== 
==1102== I   refs:      21,371,592
==1102== I1  misses:         1,244
==1102== LLi misses:         1,220
==1102== I1  miss rate:       0.01%
==1102== LLi miss rate:       0.01%
==1102== 
==1102== D   refs:       8,965,579  (5,904,097 rd   + 3,061,482 wr)
==1102== D1  misses:       847,456  (  589,912 rd   +   257,544 wr)
==1102== LLd misses:        28,976  (    2,120 rd   +    26,856 wr)
==1102== D1  miss rate:        9.5% (     10.0%     +       8.4%  )
==1102== LLd miss rate:        0.3% (      0.0%     +       0.9%  )
==1102== 
==1102== LL refs:          848,700  (  591,156 rd   +   257,544 wr)
==1102== LL misses:         30,196  (    3,340 rd   +    26,856 wr)
==1102== LL miss rate:         0.1% (      0.0%     +       0.9%  )
```

# baseline array, n = 12
```js
Table size is 2^n. Enter n ( default n=8 )  ? 12
Filling hashmap with 4014 entries
Done !
 >  > rm
 > fill
 > sum
sum : 8054091 | 8054091
 > exit
==1239== 
==1239== I   refs:      245,036
==1239== I1  misses:      1,143
==1239== LLi misses:      1,123
==1239== I1  miss rate:    0.47%
==1239== LLi miss rate:    0.46%
==1239== 
==1239== D   refs:       86,053  (59,390 rd   + 26,663 wr)
==1239== D1  misses:      7,596  ( 3,887 rd   +  3,709 wr)
==1239== LLd misses:      3,659  ( 2,099 rd   +  1,560 wr)
==1239== D1  miss rate:     8.8% (   6.5%     +   13.9%  )
==1239== LLd miss rate:     4.3% (   3.5%     +    5.9%  )
==1239== 
==1239== LL refs:         8,739  ( 5,030 rd   +  3,709 wr)
==1239== LL misses:       4,782  ( 3,222 rd   +  1,560 wr)
==1239== LL miss rate:      1.4% (   1.1%     +    5.9%  )
```

# reduce_fibo cache-profiling branch, n = 12
```js
Table size is 2^n. Enter n ( default n=8 )  ? 12
Filling hashmap with 4014 entries
hmap->top : 0
Done !
hmap->top : 4014
 >  > rm
hmap->top : 0
 > fill
hmap->top : 4014
 > sums
sum_key             : 8054091
sum_value           : 8054091
 > exit
==1178== 
==1178== I   refs:      1,522,581
==1178== I1  misses:        1,230
==1178== LLi misses:        1,207
==1178== I1  miss rate:      0.08%
==1178== LLi miss rate:      0.08%
==1178== 
==1178== D   refs:        633,131  (416,493 rd   + 216,638 wr)
==1178== D1  misses:       21,084  ( 10,879 rd   +  10,205 wr)
==1178== LLd misses:        4,327  (  2,120 rd   +   2,207 wr)
==1178== D1  miss rate:       3.3% (    2.6%     +     4.7%  )
==1178== LLd miss rate:       0.7% (    0.5%     +     1.0%  )
==1178== 
==1178== LL refs:          22,314  ( 12,109 rd   +  10,205 wr)
==1178== LL misses:         5,534  (  3,327 rd   +   2,207 wr)
==1178== LL miss rate:        0.3% (    0.2%     +     1.0%  )
```

# baseline array, n = 23
```js
Table size is 2^n. Enter n ( default n=8 )  ? 23
Filling hashmap with 8220835 entries
Done !
 >  > rm
 > fill
 > sum
sum : 33791059938195 | 33791059938195
 > exit
==1307== 
==1307== I   refs:      94,738,773
==1307== I1  misses:         1,137
==1307== LLi misses:         1,127
==1307== I1  miss rate:       0.00%
==1307== LLi miss rate:       0.00%
==1307== 
==1307== D   refs:      37,061,802  (16,493,069 rd   + 20,568,733 wr)
==1307== D1  misses:     8,224,409  ( 2,058,088 rd   +  6,166,321 wr)
==1307== LLd misses:     8,223,775  ( 2,057,530 rd   +  6,166,245 wr)
==1307== D1  miss rate:       22.2% (      12.5%     +       30.0%  )
==1307== LLd miss rate:       22.2% (      12.5%     +       30.0%  )
==1307== 
==1307== LL refs:        8,225,546  ( 2,059,225 rd   +  6,166,321 wr)
==1307== LL misses:      8,224,902  ( 2,058,657 rd   +  6,166,245 wr)
==1307== LL miss rate:         6.2% (       1.9%     +       30.0%  )
```

# reduce_fibo cache-profiling branch, n = 23
```js
Table size is 2^n. Enter n ( default n=8 )  ? 23
Filling hashmap with 8220835 entries
hmap->top : 0
Done !
hmap->top : 8220835
 >  > rm
hmap->top : 0
 > fill
hmap->top : 8220835
 > sums
sum_key             : 33791059938195
sum_value           : 33791059938195
 > exit
==1481== 
==1481== I   refs:      2,622,039,551
==1481== I1  misses:            1,220
==1481== LLi misses:            1,211
==1481== I1  miss rate:          0.00%
==1481== LLi miss rate:          0.00%
==1481== 
==1481== D   refs:      1,126,018,374  (730,561,577 rd   + 395,456,797 wr)
==1481== D1  misses:       93,955,605  ( 61,148,100 rd   +  32,807,505 wr)
==1481== LLd misses:       92,307,985  ( 59,527,535 rd   +  32,780,450 wr)
==1481== D1  miss rate:           8.3% (        8.4%     +         8.3%  )
==1481== LLd miss rate:           8.2% (        8.1%     +         8.3%  )
==1481== 
==1481== LL refs:          93,956,825  ( 61,149,320 rd   +  32,807,505 wr)
==1481== LL misses:        92,309,196  ( 59,528,746 rd   +  32,780,450 wr)
==1481== LL miss rate:            2.5% (        1.8%     +         8.3%  )
```

# reduce_fibo cache-profiling branch, SSE w PROBE_LENGTH = 16, n = 23
```js
Table size is 2^n. Enter n ( default n=8 )  ? 23
Filling hashmap with 8220835 entries
hmap->top : 0
Done !
hmap->top : 8220835
 >  > rm
hmap->top : 0
 > fill
hmap->top : 8220835
 > sums
sum_key             : 33791059938195
sum_value           : 33791059938195
 > exit
==2127== 
==2127== I   refs:      2,638,462,016
==2127== I1  misses:            1,214
==2127== LLi misses:            1,205
==2127== I1  miss rate:          0.00%
==2127== LLi miss rate:          0.00%
==2127== 
==2127== D   refs:      1,035,513,070  (672,976,312 rd   + 362,536,758 wr)
==2127== D1  misses:       93,955,395  ( 61,147,955 rd   +  32,807,440 wr)
==2127== LLd misses:       92,105,990  ( 59,327,047 rd   +  32,778,943 wr)
==2127== D1  miss rate:           9.1% (        9.1%     +         9.0%  )
==2127== LLd miss rate:           8.9% (        8.8%     +         9.0%  )
==2127== 
==2127== LL refs:          93,956,609  ( 61,149,169 rd   +  32,807,440 wr)
==2127== LL misses:        92,107,195  ( 59,328,252 rd   +  32,778,943 wr)
==2127== LL miss rate:            2.5% (        1.8%     +         9.0%  )
```

# baseline array, n = 16
```js
Table size is 2^n. Enter n ( default n=8 )  ? 16
Filling hashmap with 64225 entries
Done !
 >  > rm
 > fill
 > sum
sum : 2062393200 | 2062393200
 > rm
 > fill
 > sum
sum : 2062393200 | 2062393200
 > rm
 > fill
 > sum
sum : 2062393200 | 2062393200
 > exit
==1870== 
==1870== I   refs:      2,133,398
==1870== I1  misses:        1,135
==1870== LLi misses:        1,115
==1870== I1  miss rate:      0.05%
==1870== LLi miss rate:      0.05%
==1870== 
==1870== D   refs:        873,675  (438,578 rd   + 435,097 wr)
==1870== D1  misses:      164,341  ( 51,192 rd   + 113,149 wr)
==1870== LLd misses:       18,711  (  2,099 rd   +  16,612 wr)
==1870== D1  miss rate:      18.8% (   11.7%     +    26.0%  )
==1870== LLd miss rate:       2.1% (    0.5%     +     3.8%  )
==1870== 
==1870== LL refs:         165,476  ( 52,327 rd   + 113,149 wr)
==1870== LL misses:        19,826  (  3,214 rd   +  16,612 wr)
==1870== LL miss rate:        0.7% (    0.1%     +     3.8%  )
```

# reduce_fibo cache-profiling branch, n = 16
```js
Table size is 2^n. Enter n ( default n=8 )  ? 16
Filling hashmap with 64225 entries
hmap->top : 0
Done !
hmap->top : 64225
 >  > rm
hmap->top : 0
 > fill
hmap->top : 64225
 > sums
sum_key             : 2062393200
sum_value           : 2062393200
 > rm
hmap->top : 0
 > fill
hmap->top : 64225
 > sums
sum_key             : 2062393200
sum_value           : 2062393200
 > rm
hmap->top : 0
 > fill
hmap->top : 64225
 > sums
sum_key             : 2062393200
sum_value           : 2062393200
 > exit
==1915== 
==1915== I   refs:      51,248,956
==1915== I1  misses:         1,244
==1915== LLi misses:         1,220
==1915== I1  miss rate:       0.00%
==1915== LLi miss rate:       0.00%
==1915== 
==1915== D   refs:      21,020,549  (14,078,793 rd   + 6,941,756 wr)
==1915== D1  misses:     1,993,312  ( 1,480,016 rd   +   513,296 wr)
==1915== LLd misses:        28,976  (     2,120 rd   +    26,856 wr)
==1915== D1  miss rate:        9.5% (      10.5%     +       7.4%  )
==1915== LLd miss rate:        0.1% (       0.0%     +       0.4%  )
==1915== 
==1915== LL refs:        1,994,556  ( 1,481,260 rd   +   513,296 wr)
==1915== LL misses:         30,196  (     3,340 rd   +    26,856 wr)
==1915== LL miss rate:         0.0% (       0.0%     +       0.4%  )
```

# reduce_fibo cache-profiling branch, SSE w PROBE_LENGTH = 16, n = 16
```js
Table size is 2^n. Enter n ( default n=8 )  ? 16
Filling hashmap with 64225 entries
hmap->top : 0
Done !
hmap->top : 64225
 >  > rm
hmap->top : 0
 > fill
hmap->top : 64225
 > sums
sum_key             : 2062393200
sum_value           : 2062393200
 > rm
hmap->top : 0
 > fill
hmap->top : 64225
 > sums
sum_key             : 2062393200
sum_value           : 2062393200
 > rm
hmap->top : 0
 > fill
hmap->top : 64225
 > sums
sum_key             : 2062393200
sum_value           : 2062393200
 > exit
==2038== 
==2038== I   refs:      51,612,957
==2038== I1  misses:         1,239
==2038== LLi misses:         1,215
==2038== I1  miss rate:       0.00%
==2038== LLi miss rate:       0.00%
==2038== 
==2038== D   refs:      19,422,583  (12,995,028 rd   + 6,427,555 wr)
==2038== D1  misses:     1,915,075  ( 1,401,825 rd   +   513,250 wr)
==2038== LLd misses:        28,974  (     2,119 rd   +    26,855 wr)
==2038== D1  miss rate:        9.9% (      10.8%     +       8.0%  )
==2038== LLd miss rate:        0.1% (       0.0%     +       0.4%  )
==2038== 
==2038== LL refs:        1,916,314  ( 1,403,064 rd   +   513,250 wr)
==2038== LL misses:         30,189  (     3,334 rd   +    26,855 wr)
==2038== LL miss rate:         0.0% (       0.0%     +       0.4%  )
```


# reduce_fibo cache-profiling branch, n = 8
```js
Table size is 2^n. Enter n ( default n=8 )  ? 8
Filling hashmap with 250 entries
hmap->top : 0
Done !
hmap->top : 250
 >  > find
sum : 31125
 > exit
==2487== 
==2487== I   refs:      284,700
==2487== I1  misses:      1,211
==2487== LLi misses:      1,190
==2487== I1  miss rate:    0.43%
==2487== LLi miss rate:    0.42%
==2487== 
==2487== D   refs:      102,923  (75,232 rd   + 27,691 wr)
==2487== D1  misses:      3,521  ( 2,773 rd   +    748 wr)
==2487== LLd misses:      2,784  ( 2,118 rd   +    666 wr)
==2487== D1  miss rate:     3.4% (   3.7%     +    2.7%  )
==2487== LLd miss rate:     2.7% (   2.8%     +    2.4%  )
==2487== 
==2487== LL refs:         4,732  ( 3,984 rd   +    748 wr)
==2487== LL misses:       3,974  ( 3,308 rd   +    666 wr)
==2487== LL miss rate:      1.0% (   0.9%     +    2.4%  )
```

# baseline array, n = 8
```js
(base) [spagbol@HADES-Z: ...rce/C/build-your-own-lisp] [cache-profiling|✚ 3⚑ 1] ✔
08:00 $ valgrind --tool=cachegrind bin/baseline
==2533== Cachegrind, a cache and branch-prediction profiler
==2533== Copyright (C) 2002-2015, and GNU GPL'd, by Nicholas Nethercote et al.
==2533== Using Valgrind-3.12.0.SVN and LibVEX; rerun with -h for copyright info
==2533== Command: bin/baseline
==2533== 
--2533-- warning: L3 cache found, using its data for the LL simulation.
Table size is 2^n. Enter n ( default n=8 )  ? 8   
Filling hashmap with 250 entries
Done !
 >  > find
sum : 31125
 > exit
==2533== 
==2533== I   refs:      199,534
==2533== I1  misses:      1,137
==2533== LLi misses:      1,116
==2533== I1  miss rate:    0.57%
==2533== LLi miss rate:    0.56%
==2533== 
==2533== D   refs:       67,830  (51,347 rd   + 16,483 wr)
==2533== D1  misses:      3,408  ( 2,708 rd   +    700 wr)
==2533== LLd misses:      2,717  ( 2,098 rd   +    619 wr)
==2533== D1  miss rate:     5.0% (   5.3%     +    4.2%  )
==2533== LLd miss rate:     4.0% (   4.1%     +    3.8%  )
==2533== 
==2533== LL refs:         4,545  ( 3,845 rd   +    700 wr)
==2533== LL misses:       3,833  ( 3,214 rd   +    619 wr)
==2533== LL miss rate:      1.4% (   1.3%     +    3.8%  )
```

# reduce_fibo cache-profiling branch, n = 23
```js
Table size is 2^n. Enter n ( default n=8 )  ? 23
Filling hashmap with 8220835 entries
hmap->top : 0
Done !
hmap->top : 8220835
 >  > find
sum : 33791059938195
 > exit
==2648== 
==2648== I   refs:      1,048,825,424
==2648== I1  misses:            1,197
==2648== LLi misses:            1,188
==2648== I1  miss rate:          0.00%
==2648== LLi miss rate:          0.00%
==2648== 
==2648== D   refs:        464,014,420  (312,957,970 rd   + 151,056,450 wr)
==2648== D1  misses:       45,735,473  ( 29,330,068 rd   +  16,405,405 wr)
==2648== LLd misses:       45,695,761  ( 29,290,433 rd   +  16,405,328 wr)
==2648== D1  miss rate:           9.9% (        9.4%     +        10.9%  )
==2648== LLd miss rate:           9.8% (        9.4%     +        10.9%  )
==2648== 
==2648== LL refs:          45,736,670  ( 29,331,265 rd   +  16,405,405 wr)
==2648== LL misses:        45,696,949  ( 29,291,621 rd   +  16,405,328 wr)
==2648== LL miss rate:            3.0% (        2.2%     +        10.9%  )
```

# baseline array, n = 23
```js
Table size is 2^n. Enter n ( default n=8 )  ? 23
Filling hashmap with 8220835 entries
Done !
 >  > find
sum : 33791059938195
 > exit
==2579== 
==2579== I   refs:      92,681,388
==2579== I1  misses:         1,131
==2579== LLi misses:         1,121
==2579== I1  miss rate:       0.00%
==2579== LLi miss rate:       0.00%
==2579== 
==2579== D   refs:      20,619,344  (16,492,554 rd   + 4,126,790 wr)
==2579== D1  misses:     4,113,937  ( 2,058,046 rd   + 2,055,891 wr)
==2579== LLd misses:     4,113,303  ( 2,057,488 rd   + 2,055,815 wr)
==2579== D1  miss rate:       20.0% (      12.5%     +      49.8%  )
==2579== LLd miss rate:       19.9% (      12.5%     +      49.8%  )
==2579== 
==2579== LL refs:        4,115,068  ( 2,059,177 rd   + 2,055,891 wr)
==2579== LL misses:      4,114,424  ( 2,058,609 rd   + 2,055,815 wr)
==2579== LL miss rate:         3.6% (       1.9%     +      49.8%  )
```

```js
05:15 $ valgrind --tool=cachegrind bin/baseline
Table size is 2^n. Enter n ( default n=8 )  ? 16
Enter desired load factor ? 1
load_factor = 1.000000
load_count  = 65536
Filling hashmap with 65536 entries
Done !
|><| 0.012528 s
 > |><| 0.000397 s
 > find
sum : 2147450880
sum : 0
sum : 2146915890
|><| 115.736816 s
 > exit
==9729== 
==9729== I   refs:      45,098,341,078
==9729== I1  misses:             1,404
==9729== LLi misses:             1,352
==9729== I1  miss rate:           0.00%
==9729== LLi miss rate:           0.00%
==9729==
==9729== D   refs:       6,443,927,785  (6,443,382,890 rd   + 544,895 wr)
==9729== D1  misses:     1,609,351,908  (1,609,271,283 rd   +  80,625 wr)
==9729== LLd misses:            19,067  (        2,125 rd   +  16,942 wr)
==9729== D1  miss rate:           25.0% (         25.0%     +    14.8%  )
==9729== LLd miss rate:            0.0% (          0.0%     +     3.1%  )
==9729== 
==9729== LL refs:        1,609,353,312  (1,609,272,687 rd   +  80,625 wr)
==9729== LL misses:             20,419  (        3,477 rd   +  16,942 wr)
==9729== LL miss rate:             0.0% (          0.0%     +     3.1%  )


05:17 $ valgrind --tool=cachegrind bin/hashmap 
__AVX__ 1
PROBE_LENGTH 32
__WORDSIZE 64
RAND_MAX  2147483647
Table size is 2^n. Enter n ( default n = 8 )  ? 16
Enter desired load factor ? 1
load_factor = 1.000000
load_count  = 65536
Filling hashmap with 65536 entries
hmap->top : 0
Done !
hmap->top : 65536
|><| 0.051139 s
 > |><| 0.000560 s
 > find
sum : 2147450880
sum : 0
sum : 2144079604
|><| 0.062028 s
 > exit
==9801== 
==9801== I   refs:      19,024,896
==9801== I1  misses:         1,494
==9801== LLi misses:         1,410
==9801== I1  miss rate:       0.01%
==9801== LLi miss rate:       0.01%
==9801== 
==9801== D   refs:       7,976,713  (5,970,411 rd   + 2,006,302 wr)
==9801== D1  misses:       773,275  (  642,470 rd   +   130,805 wr)
==9801== LLd misses:        29,333  (    2,149 rd   +    27,184 wr)
==9801== D1  miss rate:        9.7% (     10.8%     +       6.5%  )
==9801== LLd miss rate:        0.4% (      0.0%     +       1.4%  )
==9801== 
==9801== LL refs:          774,769  (  643,964 rd   +   130,805 wr)
==9801== LL misses:         30,743  (    3,559 rd   +    27,184 wr)
==9801== LL miss rate:         0.1% (      0.0%     +       1.4%  )

05:25 $ valgrind --tool=cachegrind bin/hashmap
__AVX__ 1
PROBE_LENGTH 32
__WORDSIZE 64
RAND_MAX  2147483647
Table size is 2^n. Enter n ( default n = 8 )  ? 24
Enter desired load factor ? 1
load_factor = 1.000000
load_count  = 16777216
Filling hashmap with 16777216 entries
hmap->top : 0
Done !
hmap->top : 16777216
|><| 13.900773 s
 > |><| 0.000460 s
 > find
sum : 140737479966720
sum : 0
sum : 140719393292824
|><| 26.304258 s
 > exit
==9939== 
==9939== I   refs:      5,066,074,404
==9939== I1  misses:            1,502
==9939== LLi misses:            1,491
==9939== I1  miss rate:          0.00%
==9939== LLi miss rate:          0.00%
==9939== 
==9939== D   refs:      2,176,192,801  (1,622,744,525 rd   + 553,448,276 wr)
==9939== D1  misses:      187,661,670  (  154,624,975 rd   +  33,036,695 wr)
==9939== LLd misses:      184,260,468  (  151,224,349 rd   +  33,036,119 wr)
==9939== D1  miss rate:           8.6% (          9.5%     +         6.0%  )
==9939== LLd miss rate:           8.5% (          9.3%     +         6.0%  )
==9939== 
==9939== LL refs:         187,663,172  (  154,626,477 rd   +  33,036,695 wr)
==9939== LL misses:       184,261,959  (  151,225,840 rd   +  33,036,119 wr)
==9939== LL miss rate:            2.5% (          2.3%     +         6.0%  )
```