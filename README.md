
# Overview

This is meant to be an alternate implementation of a hash map, or `unordered_map` in C++, which is intended to be thread safe.  

This works for keys that can be casted to a `size_t`, such as integers, and is faster than unordered_map for keys less than 2^16. 

# Benefits

## Speed

All operations performed are on the order of θ(log(n)).
For keys with smaller values (~2<sup>16</sup>) operations are performed faster than with `std::unordered_map`. See Benchmark for details.

## Thread Safety

The concurrent version of the map makes use of atomics to allow multiple threads to simultaneously insert and obtain values.

## No resizing

This map creates new nodes an needed, and no resizing is required, unlike some hash maps. This means any pointers directly referencing values in the tree would always be valid.


# Implementation

This implements a binary tree similar to a Suffix tree.  
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

The tree can be 'squished' by increasing the number of children each node has. In this case pairs of 2 bits are obtained from the key to traverse the tree.

```
                root
       00/  01/  10\   11\
        *     *    *      *
```

At the same time, each node can be masde to store multiple values.

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

# Benchmark

The different maps were benchmarked using [Google Benchmark](https://github.com/google/benchmark). The code was compiled with gcc 10.2.0 using `-O3` and `-std=c++11` flags and linked to the benchmark libraries and openMP for parallel testing.  

In each test, unique random keys of a certain bit length were generated and inserted into the map. The total number of keys used were equal to the number that could fit within the number of bits tested (i.e. 65536 keys for 16-bit numbers) or 1 million keys, whichever was smaller. The time taken to insert the keys into the map, both with and without preallocation, as well as the time taken to read back stored values, was benchmarked.  

Comments have been added to the output shown below for clarity

## Preallocation

The speed of inserting values into `BinMap` does significantly change when space is preallocated for it, compared to `unordered_map` which needs to rehash keys when too many are inserted. This means that `BinMap` performs better when the total number of keys is not known, since no key inserted will ever cause a rehash of the map.

```
```
