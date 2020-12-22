/**             Copyright Sherwin da Cruz 2020.  
 *    https://github.com/sherwin-dc/cpp-binary-prefix-tree
 *   Distributed under the Boost Software License, Version 1.0.
 *         (See accompanying file LICENSE or copy at
 *           https://www.boost.org/LICENSE_1_0.txt)
 */


#define BINMAP_BITS 3
#include "ConcurrentBinTree.hpp"
#include "SimpleBinTree.hpp"
#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <unordered_set>
#include <unordered_map>
#include <benchmark/benchmark.h> // Google benchmark. https://github.com/google/benchmark

#define NUM_KEYS 1e6
#define SETUP                                                                         \
  std::vector<uint64_t> numbers;                                                      \
  size_t num_keys = NUM_KEYS;                                                         \
  /* no of keys to generate would be smaller of (2^bit_size or NUM_KEYS)   */         \
  num_keys = num_keys > pow(2, state.range(0)) ? pow(2, state.range(0)) : num_keys;   \
  numbers.reserve(num_keys);                                                          \
  GenerateRandom(numbers, num_keys, state.range(0), 0);                               \

// Generates unique random numbers
void GenerateRandom(std::vector<uint64_t> &result, size_t num_keys, size_t bits, size_t seed) {

  std::unordered_set<uint64_t> set;
  set.reserve(1.2 * num_keys);
  uint64_t num;

  std::mt19937_64 rng(seed);

  while (result.size() < num_keys) {
    num = rng() >> (64-bits);
    if (set.insert(num).second) {
      result.push_back(num);
    }
  }
}



static void BM_std_unordered_map_insert_reserve(benchmark::State& state) {
  // Perform setup here
  SETUP;

  for (auto _ : state) {
    // This code gets timed
    
    state.PauseTiming();
    std::unordered_map<uint64_t, uint64_t> map;
    map.reserve(1.2*num_keys);
    state.ResumeTiming();


    for (size_t i=0; i<numbers.size(); i++) {
      map[numbers[i]] = i;
    }
  }

}

static void BM_std_unordered_map_insert_no_reserve(benchmark::State& state) {
  // Perform setup here
  SETUP;

  for (auto _ : state) {
    // This code gets timed
    state.PauseTiming();
    std::unordered_map<uint64_t, uint64_t> map;
    state.ResumeTiming();

    for (size_t i=0; i<numbers.size(); i++) {
      map[numbers[i]] = i;
    }
  }

}



static void BM_std_unordered_map_read(benchmark::State& state) {
  // Perform setup here
  SETUP;

  std::unordered_map<uint64_t, uint64_t> map;
  map.reserve(1.2*num_keys);

  for (size_t i=0; i<numbers.size(); i++) {
    map[numbers[i]] = i;
  }

  for (auto _ : state) {
    // This code gets timed
    uint64_t value;
    for (size_t i=0; i<numbers.size(); i++) {
      benchmark::DoNotOptimize(value = map[numbers[i]]);
    }
  }

}

static void BM_ConcurrentBinMap_insert_no_reserve(benchmark::State& state) {
  // Perform setup here
  SETUP;


  for (auto _ : state) {
    
    state.PauseTiming();
    BinTree::ConcurrentBinMap<uint64_t> map;
    state.ResumeTiming();

    // This code gets timed
    #pragma omp parallel num_threads(4)
    { 
      #pragma omp for
      for (size_t i=0; i<numbers.size(); i++) {
        map.insert(numbers[i], i);
      }
    }
  }

}

static void BM_ConcurrentBinMap_insert_reserve(benchmark::State& state) {
  // Perform setup here
  SETUP;


  for (auto _ : state) {
    
    state.PauseTiming();
    BinTree::ConcurrentBinMap<uint64_t> map;
    for (auto num : numbers) {
      map.query(num);
    }
    state.ResumeTiming();

    // This code gets timed
    #pragma omp parallel num_threads(4)
    { 
      #pragma omp for
      for (size_t i=0; i<numbers.size(); i++) {
        map.insert(numbers[i], i);
      }
    }
  }

}

static void BM_ConcurrentBinMap_read(benchmark::State& state) {
  // Perform setup here
  SETUP;

  BinTree::ConcurrentBinMap<uint64_t> map;

  for (size_t i=0; i<numbers.size(); i++) {
    map.insert(numbers[i], i);
  }

  for (auto _ : state) {
    // This code gets timed
    #pragma omp parallel num_threads(4)
    {
      uint64_t value;
      #pragma omp for
      for (size_t i=0; i<numbers.size(); i++) {
        benchmark::DoNotOptimize(value = map.get(numbers[i]));
      }
    }
  }

}

static void BM_SimpleBinMap_insert_no_reserve(benchmark::State& state) {
  // Perform setup here
  SETUP;


  for (auto _ : state) {
    state.PauseTiming();
    BinTree::SimpleBinMap<uint64_t> map;
    state.ResumeTiming();

    // This code gets timed
    for (size_t i=0; i<numbers.size(); i++) {
      map.insert(numbers[i], i);
    }
  }

}

static void BM_SimpleBinMap_insert_reserve(benchmark::State& state) {
  // Perform setup here
  SETUP;


  for (auto _ : state) {
    state.PauseTiming();
    BinTree::SimpleBinMap<uint64_t> map;
    for (auto num : numbers) {
      map.query(num);
    }
    state.ResumeTiming();
    
    // This code gets timed
    for (size_t i=0; i<numbers.size(); i++) {
      map.insert(numbers[i], i);
    }
  }

}

static void BM_SimpleBinMap_read(benchmark::State& state) {
  // Perform setup here
  SETUP;

  BinTree::SimpleBinMap<uint64_t> map;

  for (size_t i=0; i<numbers.size(); i++) {
    map.insert(numbers[i], i);
  }

  for (auto _ : state) {
    // This code gets timed
      uint64_t value;
      for (size_t i=0; i<numbers.size(); i++) {
        benchmark::DoNotOptimize(value = map.get(numbers[i]));
      }
  }

}

// #define TESTS RangeMultiplier(2)->Range(8, 64)->UseRealTime()
// #define TESTS DenseRange(2,16,2)->UseRealTime()
#define TESTS Arg(2)->Arg(4)->Arg(6)->Arg(8)->Arg(10)->Arg(12)->Arg(14)->Arg(16)->Arg(32)->Arg(64)->UseRealTime()

BENCHMARK(BM_std_unordered_map_insert_no_reserve)->TESTS;
BENCHMARK(BM_std_unordered_map_insert_reserve)->TESTS;
BENCHMARK(BM_std_unordered_map_read)->TESTS;

BENCHMARK(BM_ConcurrentBinMap_insert_no_reserve)->TESTS;
BENCHMARK(BM_ConcurrentBinMap_insert_reserve)->TESTS;
BENCHMARK(BM_ConcurrentBinMap_read)->TESTS;

BENCHMARK(BM_SimpleBinMap_insert_no_reserve)->TESTS;
BENCHMARK(BM_SimpleBinMap_insert_reserve)->TESTS;
BENCHMARK(BM_SimpleBinMap_read)->TESTS;

BENCHMARK_MAIN();