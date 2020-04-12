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

# reduce_fibo cache-profiling branch, n = 12
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
