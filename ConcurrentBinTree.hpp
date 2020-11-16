#define BINMAP_BITS 3

#include <atomic>
#include <cstddef>

#ifndef BINMAP_BITS
  #error "Width of Map not defined. Before inclusion or at top of the file, define with '#define BINMAP_BITS n' for a node width of 2^n"
#endif


#define BINMAP_POW_1   2
#define BINMAP_POW_2   4
#define BINMAP_POW_3   8
#define BINMAP_POW_4   16
#define BINMAP_POW_5   32
#define BINMAP_POW_6   64
#define BINMAP_POW_7   128
#define BINMAP_POW_8   256
#define BINMAP_POW_9   512
#define BINMAP_POW_10  1024

#define BINMAP_POW_(x) BINMAP_POW_##x
#define BINMAP_POW(x) BINMAP_POW_(x)
#define BINMAP_WIDTH BINMAP_POW(BINMAP_BITS)

namespace BinTree {


  template<typename T>
  struct node {
    std::atomic<node<T> *> next[BINMAP_WIDTH];
    std::atomic<T> item[BINMAP_WIDTH];

    node() : next() {

    }
    ~node() {

      for (size_t i=0; i<BINMAP_WIDTH; i++) {
        delete next[i];
      }
    }
  };

  template<typename T>
  class ConcurrentBinMap {
    private:
      node<T> * root;
      void safe_traverse(node<T> * &nd, size_t key, size_t &idx);
      void quick_traverse(node<T> * &nd, size_t key, size_t &idx);
    public:
      ConcurrentBinMap();
      ~ConcurrentBinMap();
      
      /*  Stores a given key and a dynamically allocated copy of a value in the map, overiding any existing value */
      void insert(const size_t &key, const T &data);
      /*  Returns the pointer to the value stored by the key, i.e. std::atomic<value> *. 
          This is equivalent to an insert without setting a value */
      std::atomic<T> * query(const size_t &key);
      /*  Gets a value that is guarenteed to already be in the map
          This fast but unsafe if value has not already been inserted */
      const T get(const size_t &key);
  };

  template<typename T>
  ConcurrentBinMap<T>::ConcurrentBinMap() {
    root = new node<T>;
  }

  template<typename T>
  ConcurrentBinMap<T>::~ConcurrentBinMap() {
    delete root;
  }

  template<typename T>
  void ConcurrentBinMap<T>::insert(const size_t &key, const T &data) {

    node<T> * nd = root;
    size_t idx = 0;
    ConcurrentBinMap<T>::safe_traverse(nd, key, idx);

    // Will overwrite any existing data assignment
    nd->item[idx].store(data, std::memory_order_relaxed);

  }

  template<typename T>
  std::atomic<T> * ConcurrentBinMap<T>::query(const size_t &key) {
    node<T> * nd = root;
    size_t idx = 0;
    ConcurrentBinMap<T>::safe_traverse(nd, key, idx);

    // Returns pointer of node's pointer to T
    return &(nd->item[idx]);

  }

  
  template<typename T>
  const T ConcurrentBinMap<T>::get(const size_t &key) {
    node<T> * nd = root;
    size_t idx = 0;
    ConcurrentBinMap<T>::quick_traverse(nd, key, idx);

    // Return T
    return nd->item[idx].load(std::memory_order_relaxed);
  }
  
  template<typename T>
  void ConcurrentBinMap<T>::safe_traverse(node<T> * &nd, size_t key, size_t &idx) {

    node<T> * new_node;
    node<T> * expected_node;

    while (key) {
      // Get the last EXP bits
      idx = key & (BINMAP_WIDTH-1);
      key = key >> BINMAP_BITS;

      if (!key) {
        break;
      }        
      expected_node = nd->next[idx].load(std::memory_order_relaxed);
      if (expected_node == nullptr) {
        new_node = new node<T>;
        if (nd->next[idx].compare_exchange_strong(expected_node, new_node, std::memory_order_relaxed, std::memory_order_relaxed)) {
          nd = new_node;
        } else {
          nd = expected_node;
          delete new_node;
        }

      } else {
        nd = expected_node;
      }


      
    }
  }

  template<typename T>
  void ConcurrentBinMap<T>::quick_traverse(node<T> * &nd, size_t key, size_t &idx) {


    while (key) {
      // Get the last EXP bits
      idx = key & (BINMAP_WIDTH-1);
      key = key >> BINMAP_BITS;
      if (!key) {
        break;
      }
      nd = nd->next[idx].load(std::memory_order_relaxed);

    }
    
  }




}