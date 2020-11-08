
# Overview

This is meant to be an alternate implementation of a hash map, or `unordered_map` in C++, which is intended to be thread safe.  

This works for keys that can be casted to a `size_t`, such as integers.  

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

```c++

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

# Benefits

All operations performed are on the order of log<sub>2</sub>(n)
The advantages of this compared to a hash map are:

## Thread Safety

Note: This is still a work in progress and there is no guarentees that an unintended data race would not occur. If you spot any errors feel free to add a bug report to create a pull request.  
However this map has been designed with thread safety in mind and all operations on the map should be able to happen concurrently by multiple threads.

## No hash functions are used

There is no need to deal with hash functions since the keys are integral types, hence no collisions in the map can occur.
Remapping and rehashing of the tree do not occur


3. Memory is allocated as needed
