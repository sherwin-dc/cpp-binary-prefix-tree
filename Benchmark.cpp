#include "BinTree.hpp"
#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <unordered_set>
#include <unordered_map>
// Google benchmark. https://github.com/google/benchmark
#include <benchmark/benchmark.h>

#define NUM_KEYS 1e6
#define SETUP                                               \
  std::vector<uint64_t> numbers;                            \
  numbers.reserve(NUM_KEYS);                                \
  GenerateRandom(numbers, NUM_KEYS, state.range(0), 0);     \

// Generates unique random numbers
void GenerateRandom(std::vector<uint64_t> &result, size_t num_keys, size_t bits, size_t seed) {

  // number of keys to generate would be number of keys within bit size or NUM_KEYS, whichever is smaller
  num_keys = num_keys > pow(2, bits) ? pow(2, bits) : num_keys;

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



static void BM_std_unordered_map_insert(benchmark::State& state) {
  // Perform setup here
  SETUP;

  for (auto _ : state) {
    // This code gets timed
    std::unordered_map<uint64_t, uint64_t> map;
    // map.reserve(2*NUM_KEYS);

    for (size_t i=0; i<numbers.size(); i++) {
      map[numbers[i]] = i;
    }
  }

}



static void BM_std_unordered_map_read(benchmark::State& state) {
  // Perform setup here
  SETUP;

  std::unordered_map<uint64_t, uint64_t> map;
  // map.reserve(2*NUM_KEYS);

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

static void BM_ConcurrentBinMap_insert(benchmark::State& state) {
  // Perform setup here
  SETUP;

  BinTree::ConcurentBinMap<uint64_t> map;

  for (auto _ : state) {
    // This code gets timed
    #pragma omp parallel
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

  BinTree::ConcurentBinMap<uint64_t> map;

  for (size_t i=0; i<numbers.size(); i++) {
    map.insert(numbers[i], i);
  }

  for (auto _ : state) {
    // This code gets timed
    #pragma omp parallel
    {
      uint64_t value;
      #pragma omp for
      for (size_t i=0; i<numbers.size(); i++) {
        benchmark::DoNotOptimize(value = map.get(numbers[i]));
      }
    }
  }

}

#define TESTS RangeMultiplier(2)->Range(8, 64)->UseRealTime()

BENCHMARK(BM_std_unordered_map_insert)->TESTS;
BENCHMARK(BM_std_unordered_map_read)->TESTS;

BENCHMARK(BM_ConcurrentBinMap_insert)->TESTS;
BENCHMARK(BM_ConcurrentBinMap_read)->TESTS;

BENCHMARK_MAIN();