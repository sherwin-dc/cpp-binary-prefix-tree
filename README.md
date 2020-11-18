
# Overview

This is meant to be an alternate implementation of a hash map, or `unordered_map` in C++, which is intended to be thread safe.  

This works for keys that can be casted to a `size_t`, such as integers, and is faster than unordered_map for keys less than 2<sup>16</sup>. 

# Benefits

## Speed

Insering and retrieving values are on the order of O(log(n)).
For keys with smaller values (~2<sup>16</sup>) operations are performed faster than with `std::unordered_map`. See [Benchmarks](#benchmarks) for details.

## Thread Safety

The concurrent version of `BinMap` makes use of atomics to allow multiple threads to simultaneously insert and obtain values.

## No resizing

`BinMap` creates new nodes an needed, and no resizing is required, unlike some hash maps. This means any pointers directly referencing values in the tree would always be valid.

## Flexibility

There is a trade off between speed and memory usage, which the user can control at compile time by specifying the size of each node. For example, a tree with each node having 2 children would use less memory but perform slower than a tree with each node having 16 children.

# Usage

Simply `#include` the header files that you need. Be sure to define the [bit width](#changing-node-size) of the tree with `#define BINMAP_BITS` before including the header.

## Single threaded

```c++
#define BINMAP_BITS 4                               // specify bit width of tree
#include "SimpleBinTree.hpp"
...

int main() {
  // Simple Bin Map
    BinTree::SimpleBinMap<std::string> map;         // map of int keys to string values

    map.insert(5, "foo");                           // insert key-value pair
    assert(map.get(5) == "foo");                    // obtain value using key. ensure that (i, value) already exists in the map first!
    map.get(5) = "bar"                              // changing an existing key using assignment
    assert(map.get(5) == "bar");

    std::string * ptr = map.query(6);               // reserving space for the key '6', and also obtaining pointer to the uninitialised value in the map
    *ptr = "baz";
    assert(map.get(6) == "baz");
}
```

## Multithreaded

```c++
#define BINMAP_BITS 4                               // specify bit width of tree
#include "ConcurrentBinTree.hpp"
#include <atomic>
...

int main() {
  // Concurrent Bin Map
    BinTree::ConcurrentBinMap<std::string> map;
    ...

    /* Inside multiple threads */
    map.insert(i, value);                           // any existing value associated with the key would be overwritten, possibly by another thread
    value = map.get(i);                             // ensure that (i, value) already exists in the map first!

    std::atomic<std::string> * ptr = map.query(j);  // using query() to obtain pointer to atomic value before modifying it
    *ptr.store(value, std::memory_order);


    /* Idle thread reserving space */
    map.query(j);

    /* Another thread inputing values */
    map.insert(j, value);

}
```

# Implementation

## Naive implementation

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

## Changing node size

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

As the size of each node increases, the structure becomes closer to that of a direct access table, and memory usage would also increase. The width of each node in the tree can be determined at compile time by specifying the *bit width* of the tree, which means that many bits are at each node to index into the next node or to store the value. The width of each node then becomes 2<sup>bit width</sup>.

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

In this test the benchmark for a bit width of 3 was rerun, but with openMP disabled by not linking to its library during compilation. For the tests with openMP enabled, 4 threads were used.

### Inserting values

<details>

<summary>View benchmark results</summary>

```c++
----------------------------------------------------------------------------------------------
Benchmark                                                    Time             CPU   Iterations
----------------------------------------------------------------------------------------------

/* unordered_map */
BM_std_unordered_map_insert_reserve/2/real_time            291 ns          293 ns      2159667
BM_std_unordered_map_insert_reserve/4/real_time            521 ns          518 ns      1392193
BM_std_unordered_map_insert_reserve/6/real_time           1553 ns         1548 ns       461105
BM_std_unordered_map_insert_reserve/8/real_time           8923 ns         8920 ns        72995
BM_std_unordered_map_insert_reserve/10/real_time         31393 ns        31395 ns        21207
BM_std_unordered_map_insert_reserve/12/real_time        124764 ns       124823 ns         5625
BM_std_unordered_map_insert_reserve/14/real_time        617406 ns       617921 ns         1282
BM_std_unordered_map_insert_reserve/16/real_time       2461009 ns      2462134 ns          288
BM_std_unordered_map_insert_reserve/32/real_time     215482200 ns    215479591 ns            3
BM_std_unordered_map_insert_reserve/64/real_time     194774850 ns    194774261 ns            4

/* SimpleBinMap (BINMAP_BITS=3) */
BM_SimpleBinMap_insert_no_reserve/2/real_time              246 ns          248 ns      3247610
BM_SimpleBinMap_insert_no_reserve/4/real_time              405 ns          407 ns      1663915
BM_SimpleBinMap_insert_no_reserve/6/real_time              543 ns          546 ns      1281282
BM_SimpleBinMap_insert_no_reserve/8/real_time             4155 ns         4159 ns       165713
BM_SimpleBinMap_insert_no_reserve/10/real_time           32594 ns        32672 ns        24810
BM_SimpleBinMap_insert_no_reserve/12/real_time           41186 ns        41215 ns        17167
BM_SimpleBinMap_insert_no_reserve/14/real_time          384593 ns       384875 ns         1856
BM_SimpleBinMap_insert_no_reserve/16/real_time         2782199 ns      2784006 ns          248
BM_SimpleBinMap_insert_no_reserve/32/real_time       924104800 ns    924093460 ns            1
BM_SimpleBinMap_insert_no_reserve/64/real_time      2384347600 ns   2384320414 ns            1

/* ConcurrentBinMap (BINMAP_BITS=3) with openMP turned off */
BM_ConcurrentBinMap_insert_no_reserve/2/real_time          223 ns          227 ns      3298776
BM_ConcurrentBinMap_insert_no_reserve/4/real_time          491 ns          497 ns      1448631
BM_ConcurrentBinMap_insert_no_reserve/6/real_time          608 ns          611 ns      1109454
BM_ConcurrentBinMap_insert_no_reserve/8/real_time         4752 ns         4753 ns       152355
BM_ConcurrentBinMap_insert_no_reserve/10/real_time       28922 ns        28954 ns        19419
BM_ConcurrentBinMap_insert_no_reserve/12/real_time       43351 ns        43384 ns        16680
BM_ConcurrentBinMap_insert_no_reserve/14/real_time      385086 ns       385232 ns         1761
BM_ConcurrentBinMap_insert_no_reserve/16/real_time     3021692 ns      3023652 ns          240
BM_ConcurrentBinMap_insert_no_reserve/32/real_time   973621500 ns    973613279 ns            1
BM_ConcurrentBinMap_insert_no_reserve/64/real_time  2434763400 ns   2434738849 ns            1

/* ConcurrentBinMap (BINMAP_BITS=3) with openMP turned on */
BM_ConcurrentBinMap_insert_no_reserve/2/real_time         1616 ns         1630 ns       588302
BM_ConcurrentBinMap_insert_no_reserve/4/real_time         2861 ns         2887 ns       244370
BM_ConcurrentBinMap_insert_no_reserve/6/real_time         3126 ns         3151 ns       213016
BM_ConcurrentBinMap_insert_no_reserve/8/real_time        10294 ns        10327 ns        72562
BM_ConcurrentBinMap_insert_no_reserve/10/real_time       55586 ns        55676 ns        10000
BM_ConcurrentBinMap_insert_no_reserve/12/real_time       65362 ns        65614 ns        11012
BM_ConcurrentBinMap_insert_no_reserve/14/real_time      578334 ns       579032 ns         1385
BM_ConcurrentBinMap_insert_no_reserve/16/real_time     3995498 ns      3997647 ns          175
BM_ConcurrentBinMap_insert_no_reserve/32/real_time  4351158400 ns   1142168315 ns            1
BM_ConcurrentBinMap_insert_no_reserve/64/real_time 13526372900 ns   3143463510 ns            1
```

</details>

### Reading values

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
BM_SimpleBinMap_read/2/real_time                          3.73 ns         3.73 ns    182693211
BM_SimpleBinMap_read/4/real_time                          19.0 ns         19.0 ns     36188540
BM_SimpleBinMap_read/6/real_time                          85.7 ns         85.7 ns      7741804
BM_SimpleBinMap_read/8/real_time                           415 ns          415 ns      1833608
BM_SimpleBinMap_read/10/real_time                         1746 ns         1746 ns       381444
BM_SimpleBinMap_read/12/real_time                         7506 ns         7506 ns        90643
BM_SimpleBinMap_read/14/real_time                        64168 ns        64167 ns        10419
BM_SimpleBinMap_read/16/real_time                       444752 ns       444745 ns         1551
BM_SimpleBinMap_read/32/real_time                     81659843 ns     81660219 ns            7
BM_SimpleBinMap_read/64/real_time                    271722800 ns    271723949 ns            2

/* ConcurrentBinMap (BINMAP_BITS=3) with openMP turned off */
BM_ConcurrentBinMap_read/2/real_time                      6.40 ns         6.40 ns     95794747
BM_ConcurrentBinMap_read/4/real_time                      20.3 ns         20.3 ns     34118414
BM_ConcurrentBinMap_read/6/real_time                      83.5 ns         83.5 ns      8418855
BM_ConcurrentBinMap_read/8/real_time                       429 ns          429 ns      1651400
BM_ConcurrentBinMap_read/10/real_time                     1849 ns         1849 ns       373060
BM_ConcurrentBinMap_read/12/real_time                     8658 ns         8658 ns        75904
BM_ConcurrentBinMap_read/14/real_time                    71516 ns        71517 ns         9371
BM_ConcurrentBinMap_read/16/real_time                   527207 ns       527207 ns         1284
BM_ConcurrentBinMap_read/32/real_time                 83285550 ns     83284901 ns            8
BM_ConcurrentBinMap_read/64/real_time                270331100 ns    270332063 ns            3

/* ConcurrentBinMap (BINMAP_BITS=3) with openMP turned on */
BM_ConcurrentBinMap_read/2/real_time                      1132 ns         1132 ns       757783
BM_ConcurrentBinMap_read/4/real_time                      1073 ns         1073 ns       638248
BM_ConcurrentBinMap_read/6/real_time                      1245 ns         1245 ns       550202
BM_ConcurrentBinMap_read/8/real_time                      1402 ns         1402 ns       493758
BM_ConcurrentBinMap_read/10/real_time                     2298 ns         2298 ns       352237
BM_ConcurrentBinMap_read/12/real_time                     6457 ns         6457 ns       110455
BM_ConcurrentBinMap_read/14/real_time                    39322 ns        39322 ns        14976
BM_ConcurrentBinMap_read/16/real_time                   251376 ns       251376 ns         2952 
BM_ConcurrentBinMap_read/32/real_time                 34501565 ns     29282024 ns           20    <- Fastest 32
BM_ConcurrentBinMap_read/64/real_time                114486267 ns     81944345 ns            6
```

</details>

In both cases we can see that using `ConcurrentBinMap` over `SimpleBinMap` for singlethreaded operations only exerts a small penalty on performance. However, when multiple threads are inserting values simultaneouly the performance drops dramatically. This is likely due to false sharing with each node storing multiple atomic pointers. Howeevr, the situation is much better when reading data, with `ConcurrentBinMap` reading faster with 4 threads than a singlwe threaded `SimpleBinMap` when values were greater than 12 bits. This is the point where the number of values inserted is large enough such that multiple threads are more efficient. `ConcurrentBinMap` was able to read 1 million 32 bit values with 4 threads just as fast as a single threaded `unordered_map`. The difference between inserting and reading values is that reading only uses atomic reads on variables, hence false sharing is not a problem unlike inserting values where cache lines have to be invalidated during atomic writes to variables.  

Hence `ConcurrentBinMap` is best used to read a larger number of values concurrently, but can still be used to insert values across multiple threads without the user haing to manually implement locks.

# Further work

Further work could include:
  - Overloading `operator[]` for a more natural coding style. 
  - Implementing `contains` method to check if a key exists.
  - Implementing `reserve` method that prealocates a block of keys or parts of them (for example creating all the nodes within the next x layers).
  - Implementing `remove` method to recursively remove values and free memory taken up by empty nodes.
  - Benchmarking the memory usage of each map.
  - Benchmarking `BinMap` for greater bit widths.
  - Benchmarking `BinMap` against other HashMap and ConcurrentHashMap implementations.
  - Allowing `BinMap` to use non integral data types as keys through hashing, effectively converrting this into a HashMap, but retaining the tree structure and the benefits of no resizing being required. This would then require a system of handling collisions, such as chaining by adding a linked list to each node when inserting values; or open addresiing by inserting a new child node when needed.