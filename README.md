
# Overview

This is meant to be an alternate implementation of a hash map, or `unordered_map` in C++, which is intended to be thread safe.  

This works for keys that can be casted to a `size_t`, such as integers, and is faster than unordered_map for keys less than 2<sup>16</sup>. 

# Benefits

## Speed

All operations performed are on the order of θ(log(n)).
For keys with smaller values (~2<sup>16</sup>) operations are performed faster than with `std::unordered_map`. See [Benchmarks](#benchmarks) for details.

## Thread Safety

The concurrent version of the map makes use of atomics to allow multiple threads to simultaneously insert and obtain values.

## No resizing

This map creates new nodes an needed, and no resizing is required, unlike some hash maps. This means any pointers directly referencing values in the tree would always be valid.


# Implementation

The basic idea behind `BinMap` is to implement a binary tree similar to a Suffix tree.  
When a key and value are given to the map, it looks at the binary value of the key and places the value in a leaf of the tree that can be obtained by following the binary value of the key.  
For example, given the following key value pairs:

```c++
(2, A) // key is 0b0010
(0, B) // key is 0b0000
(3, C) // key is 0b0011
(8 ,D) // Key is 0b0100
```

Each key value is read in binary from right to left and inserted in an appropriate place in the tree, as shown.  
Note that nodes in the tree are only created where needed, and nodes may have only 1 branch.  

```

         root
           *~~B
        0/  \1
        /    \ 
       *      *
     /  \      \
    *     *~~A   *~~C
     \
      *~~D
```
However, this implementation is not very cache frendly as a lot of traversing is required to reach values.

The tree can be 'squished' by increasing the number of children each node has. In this case pairs of 2 bits are obtained from the key to traverse the tree, and the length of the tree is reduced from log<sub>2</sub>(n) to log<sub>4</sub>(n).

```
                root
       00/  01/  10\   11\
        *     *    *      *
```

At the same time, each node can be made to store multiple values, such that an extra traversal is not needed just to store a single value..

```
        * 00
        |~~
        | 01
        |~~
        | 10
        |~~
        | 11
        |~~
```

In this case the tree is no longer a 'binary' tree where each node has 2 children, but still uses the binary value of the key to traverse the tree.  

The width of each node in the tree can be determined at compile time by specifying the *bit width* of the tree, which means that many bits are at each node to index into the next node or to store the value. The width of each node then becomes 2<sup>bit width</sup>.

# Benchmarks

The different maps were benchmarked using [Google Benchmark](https://github.com/google/benchmark). The code was compiled with gcc 10.2.0 using `-O3` and `-std=c++11` flags and linked to the benchmark libraries and openMP for parallel testing.  

In each test, unique random keys of a certain bit length were generated and inserted into the map. The total number of keys used were equal to the number that could fit within the number of bits tested (i.e. 65536 keys for 16-bit numbers) or 1 million keys, whichever was smaller. The time taken to insert the keys into the map, both with and without preallocation, as well as the time taken to read back stored values, was benchmarked. This was run three times, with the width of each node in `BinMap` being changed.  

All the timings from `unordered_map` are from the first run with bit width = 3. Comments have been added to the output below for clarity.

## With and without Preallocation

The speed of inserting values into `BinMap` does not significantly change when space is preallocated for it, compared to `unordered_map` which needs to rehash keys when too many are inserted. This means that `BinMap` would be preferable when the total number of keys is not known, since no key inserted will ever cause a rehash of the map.

<details>

<summary>View benchmark results</summary>

```c++
----------------------------------------------------------------------------------------------
Benchmark                                                    Time             CPU   Iterations
----------------------------------------------------------------------------------------------

/* unordered_map without preallocating space */
BM_std_unordered_map_insert_no_reserve/16/real_time    3986199 ns      3987009 ns          186
BM_std_unordered_map_insert_no_reserve/32/real_time  381567150 ns    381563115 ns            2
BM_std_unordered_map_insert_no_reserve/64/real_time  427925500 ns    427928617 ns            2

/* unordered_map with preallocating space */
BM_std_unordered_map_insert_reserve/16/real_time       2461009 ns      2462134 ns          288
BM_std_unordered_map_insert_reserve/32/real_time     215482200 ns    215479591 ns            3
BM_std_unordered_map_insert_reserve/64/real_time     194774850 ns    194774261 ns            4

/* SimpleBinMap (BINMAP_BITS=3) without preallocating space */
BM_SimpleBinMap_insert_no_reserve/16/real_time         3430044 ns      3431646 ns          185
BM_SimpleBinMap_insert_no_reserve/32/real_time      1007438600 ns   1007434578 ns            1
BM_SimpleBinMap_insert_no_reserve/64/real_time      2672090000 ns   2672071320 ns            1

/* SimpleBinMap (BINMAP_BITS=3) with preallocating space */
BM_SimpleBinMap_insert_reserve/16/real_time            2739430 ns      2740556 ns          235
BM_SimpleBinMap_insert_reserve/32/real_time          585880600 ns    585875938 ns            1
BM_SimpleBinMap_insert_reserve/64/real_time         1625717700 ns   1625697520 ns            1

/* SimpleBinMap (BINMAP_BITS=4) without preallocating space */
BM_SimpleBinMap_insert_no_reserve/16/real_time          678011 ns       678357 ns         1014
BM_SimpleBinMap_insert_no_reserve/32/real_time       951515800 ns    951493420 ns            1
BM_SimpleBinMap_insert_no_reserve/64/real_time      2668421300 ns   2668391199 ns            1

/* SimpleBinMap (BINMAP_BITS=4) with preallocating space */
BM_SimpleBinMap_insert_reserve/16/real_time             523534 ns       523578 ns         1318
BM_SimpleBinMap_insert_reserve/32/real_time          545057200 ns    545055468 ns            1
BM_SimpleBinMap_insert_reserve/64/real_time         1561955500 ns   1561941484 ns            1

/* SimpleBinMap (BINMAP_BITS=5) without preallocating space */
BM_SimpleBinMap_insert_no_reserve/16/real_time         8394130 ns      8398668 ns           82
BM_SimpleBinMap_insert_no_reserve/32/real_time      1324323500 ns   1324311676 ns            1
BM_SimpleBinMap_insert_no_reserve/64/real_time      2945760700 ns   2945697072 ns            1

/* SimpleBinMap (BINMAP_BITS=5) with preallocating space */
BM_SimpleBinMap_insert_reserve/16/real_time            7440732 ns      7442756 ns          101
BM_SimpleBinMap_insert_reserve/32/real_time          700335300 ns    700329632 ns            1
BM_SimpleBinMap_insert_reserve/64/real_time         1759430300 ns   1759401691 ns            1
```

We can see that `unordered_map` takes a huge performance hit when space is not preallocated, especially in the last 2 tests where inserting 1 million keys into an `unordered_map` without reserving space was **twice** as slow. With `BinMap`, the impact is not as drastic, and is reduced when increasing the node width. With less nodes between the root and the leaf of the tree, preallocation has a smaller impact on performance.  

Note that preallocating space itself takes time, which have not been included in the timings shown above. In any case, *for keys of 16 bits or less, `BinMap` of bit width 4 inserts keys faster without preallocation than `unordered_map` does with preallocation*.

</details>

In the subsequent sections we will only be comparing the timings of `BinMap` without preallocation to `unordered_map` with preallocation.

## Inserting values

<details>

<summary>View benchmark results</summary>

```c++
----------------------------------------------------------------------------------------------
Benchmark                                                    Time             CPU   Iterations
----------------------------------------------------------------------------------------------

/* unordered_map with preallocating space */
BM_std_unordered_map_insert_reserve/2/real_time            291 ns          293 ns      2159667
BM_std_unordered_map_insert_reserve/4/real_time            521 ns          518 ns      1392193
BM_std_unordered_map_insert_reserve/6/real_time           1553 ns         1548 ns       461105
BM_std_unordered_map_insert_reserve/8/real_time           8923 ns         8920 ns        72995
BM_std_unordered_map_insert_reserve/10/real_time         31393 ns        31395 ns        21207
BM_std_unordered_map_insert_reserve/12/real_time        124764 ns       124823 ns         5625
BM_std_unordered_map_insert_reserve/14/real_time        617406 ns       617921 ns         1282
BM_std_unordered_map_insert_reserve/16/real_time       2461009 ns      2462134 ns          288

/* SimpleBinMap (BINMAP_BITS=3) without preallocating space */
BM_SimpleBinMap_insert_no_reserve/2/real_time              246 ns          243 ns      3129755
BM_SimpleBinMap_insert_no_reserve/4/real_time              563 ns          571 ns      1052967
BM_SimpleBinMap_insert_no_reserve/6/real_time              561 ns          564 ns      1133049    <- Fastest 6
BM_SimpleBinMap_insert_no_reserve/8/real_time             4939 ns         4940 ns       140268
BM_SimpleBinMap_insert_no_reserve/10/real_time           34448 ns        34480 ns        20241
BM_SimpleBinMap_insert_no_reserve/12/real_time           49766 ns        49813 ns        13906
BM_SimpleBinMap_insert_no_reserve/14/real_time          685277 ns       686280 ns         1029
BM_SimpleBinMap_insert_no_reserve/16/real_time         3430044 ns      3431646 ns          185

/* SimpleBinMap (BINMAP_BITS=4) without preallocating space */
BM_SimpleBinMap_insert_no_reserve/2/real_time              216 ns          217 ns      3105838    <- Fastest 2
BM_SimpleBinMap_insert_no_reserve/4/real_time              249 ns          251 ns      2877634    <- Fastest 4
BM_SimpleBinMap_insert_no_reserve/6/real_time             1052 ns         1057 ns       676602
BM_SimpleBinMap_insert_no_reserve/8/real_time             1396 ns         1393 ns       511296    <- Fastest 8
BM_SimpleBinMap_insert_no_reserve/10/real_time           25682 ns        25719 ns        27386
BM_SimpleBinMap_insert_no_reserve/12/real_time           33269 ns        33309 ns        21228    <- Fastest 12
BM_SimpleBinMap_insert_no_reserve/14/real_time          540767 ns       541168 ns         1375
BM_SimpleBinMap_insert_no_reserve/16/real_time          678011 ns       678357 ns         1014    <- Fastest 16

/* SimpleBinMap (BINMAP_BITS=5) without preallocating space */
BM_SimpleBinMap_insert_no_reserve/2/real_time              297 ns          297 ns      2179729
BM_SimpleBinMap_insert_no_reserve/4/real_time              322 ns          322 ns      2162047
BM_SimpleBinMap_insert_no_reserve/6/real_time             4660 ns         4659 ns       152793
BM_SimpleBinMap_insert_no_reserve/8/real_time             5344 ns         5387 ns       129247
BM_SimpleBinMap_insert_no_reserve/10/real_time            6334 ns         6332 ns       105021    <- Fastest 10
BM_SimpleBinMap_insert_no_reserve/12/real_time          168253 ns       168396 ns         4094
BM_SimpleBinMap_insert_no_reserve/14/real_time          201212 ns       201363 ns         3434    <- Fastest 14
BM_SimpleBinMap_insert_no_reserve/16/real_time         8394130 ns      8398668 ns           82
```

Here we can compare how changing the width of each node can affect performance. It can be seen that the fastest results always occured where the maximum bits in the key was a multiple of the number of bits in each node for indexing. Therefore it is known beforehand what the distribution of the keys would be like, the number of bits used in each node can be set to minimise the depth of the tree. A bit width of 4 naturally seems to work best for numbers in the range tested.

For 8 bit values `BinMap` was about 6 times as fast as `unordered_map`, and still 3.5 as fast for 16 bit numbers.

</details>

## Reading values

<details>

<summary>View benchmark results</summary>

```c++
----------------------------------------------------------------------------------------------
Benchmark                                                    Time             CPU   Iterations
----------------------------------------------------------------------------------------------

/* unordered_map */
BM_std_unordered_map_read/2/real_time                     12.0 ns         12.0 ns     56661810
BM_std_unordered_map_read/4/real_time                     47.0 ns         47.0 ns     14976087
BM_std_unordered_map_read/6/real_time                      190 ns          190 ns      3657207
BM_std_unordered_map_read/8/real_time                     1021 ns         1021 ns       742950
BM_std_unordered_map_read/10/real_time                    3011 ns         3011 ns       224034
BM_std_unordered_map_read/12/real_time                   12313 ns        12313 ns        56468
BM_std_unordered_map_read/14/real_time                   48491 ns        48491 ns        14388
BM_std_unordered_map_read/16/real_time                  200438 ns       200435 ns         3418
BM_std_unordered_map_read/32/real_time                35317916 ns     35317848 ns           19
BM_std_unordered_map_read/64/real_time                35121370 ns     35121415 ns           20

/* SimpleBinMap (BINMAP_BITS=3) */
BM_SimpleBinMap_read/2/real_time                          8.07 ns         8.07 ns     90428878
BM_SimpleBinMap_read/4/real_time                          32.5 ns         32.5 ns     20268318
BM_SimpleBinMap_read/6/real_time                           113 ns          113 ns      5980744
BM_SimpleBinMap_read/8/real_time                           472 ns          472 ns      1664567
BM_SimpleBinMap_read/10/real_time                         1955 ns         1955 ns       333016
BM_SimpleBinMap_read/12/real_time                         9519 ns         9520 ns        80062
BM_SimpleBinMap_read/14/real_time                        70863 ns        70863 ns         9305
BM_SimpleBinMap_read/16/real_time                       480798 ns       480798 ns         1473
BM_SimpleBinMap_read/32/real_time                     82987144 ns     82986698 ns            9
BM_SimpleBinMap_read/64/real_time                    277999600 ns    278000318 ns            3

/* SimpleBinMap (BINMAP_BITS=4) */
BM_SimpleBinMap_read/2/real_time                          3.83 ns         3.83 ns    174262261
BM_SimpleBinMap_read/4/real_time                          15.2 ns         15.2 ns     47411277
BM_SimpleBinMap_read/6/real_time                          79.8 ns         79.8 ns      8782760
BM_SimpleBinMap_read/8/real_time                           400 ns          400 ns      1609938
BM_SimpleBinMap_read/10/real_time                         1529 ns         1529 ns       445545
BM_SimpleBinMap_read/12/real_time                         6350 ns         6350 ns       107322
BM_SimpleBinMap_read/14/real_time                        50576 ns        50576 ns        13803
BM_SimpleBinMap_read/16/real_time                       161474 ns       161472 ns         4359    <- Fastest 16
BM_SimpleBinMap_read/32/real_time                     57785867 ns     57785358 ns           12
BM_SimpleBinMap_read/64/real_time                    232237833 ns    232236087 ns            3

/* SimpleBinMap (BINMAP_BITS=5) */
BM_SimpleBinMap_read/2/real_time                          3.56 ns         3.56 ns    180595760 
BM_SimpleBinMap_read/4/real_time                          15.3 ns         15.3 ns     45786917
BM_SimpleBinMap_read/6/real_time                          77.7 ns         77.7 ns      9069895
BM_SimpleBinMap_read/8/real_time                           317 ns          317 ns      2230515    <- Fastest 8
BM_SimpleBinMap_read/10/real_time                         1368 ns         1368 ns       521976    <- Fastest 10
BM_SimpleBinMap_read/12/real_time                         6314 ns         6314 ns       108347    <- Fastest 12
BM_SimpleBinMap_read/14/real_time                        29143 ns        29143 ns        23129    <- Fastest 14
BM_SimpleBinMap_read/16/real_time                       377097 ns       377097 ns         1800
BM_SimpleBinMap_read/32/real_time                     54863369 ns     54862955 ns           13
BM_SimpleBinMap_read/64/real_time                    180063750 ns    180061893 ns            4
```

For reading values we can immediately see that `unordered_map` is far ahead in performance when reading 32 to 64 bit values, with little difference betwwen the 2, amounting to the O(1) lookup time it provides. The `BinMap` of node bit width 5 unexpectedly performed better than bit width 4, but only marginally.

For 8 bit values `BinMap` was about 3 times as fast than `unordered_map`, and still 1.2 times as fast for 16 bit numbers.

</details>

## Multithreaded vs Singlethreaded

# Further work

Further work could include:
  - Creating a `contains` method to check if a key exists.
  - Creating a `reserve` method that prealocates a block of keys or parts of them (for example creating all the nodes within the next x layers).
  - Benchmarking `BinMap` for greater bit widths.
  - Benchmarking `BinMap` against other HashMap and ConcurrentHashMap implementations.