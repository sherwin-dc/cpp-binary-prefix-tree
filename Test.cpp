#include "BinTree.hpp"
#include <iostream>
#include <vector>
#include <random>
#include <unordered_set>
#include <unordered_map>

#define NUM_KEYS 10000
#define BIT_MASK_16 65535

// Generates unique random numbers
void GenerateRandom(std::vector<uint64_t> &result, size_t num_keys, uint64_t mask) {

  std::unordered_set<uint64_t> set;
  set.reserve(num_keys);
  uint64_t num;

  std::random_device rd;
  std::mt19937_64 rng(rd());

  while (result.size() < num_keys) {
    num = rng() & mask;
    if (set.insert(num).second) {
      result.push_back(num);
    }
  }
}

// Test data type to store in map
struct MyStruct {
  uint64_t foo;
  MyStruct(uint64_t init=0) {
    foo = init;
  }
};

int main() {

  // Generate 10,000 unique random numbers
  std::vector<uint64_t> numbers;
  numbers.reserve(NUM_KEYS);
  
  GenerateRandom(numbers, NUM_KEYS, BIT_MASK_16);

  // Print out the data once
  std::unordered_map<uint64_t, MyStruct> stlmap;
  for (size_t i = 0; i < numbers.size(); i++) {
    std::cout << numbers[i] << " ";
  }

  std::cout << std::endl;



}