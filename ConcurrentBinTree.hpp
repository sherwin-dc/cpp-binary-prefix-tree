#define BINMAP_EXP 3

#include <atomic>
#include <cstddef>

#ifndef BINMAP_EXP
  #error "Width of Map not defined. Before inclusion or at top of the file, define with '#define BINMAP_EXP n' for a node width of 2^n"
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
#define BINMAP_WIDTH BINMAP_POW(BINMAP_EXP)

namespace BinTree {


  template<typename T>
  struct node {
    std::atomic<node<T> *> next[BINMAP_WIDTH];
    std::atomic<T *> item;

    node() : next() {
      item = nullptr;
    }
    ~node() {
      delete item;
      for (size_t i=0; i<BINMAP_WIDTH; i++) {
        delete next[i];
      }
    }
  };

  template<typename T>
  class ConcurrentBinMap {
    private:
      node<T> * root;
      void safe_traverse(node<T> * &nd, size_t key);
      void quick_traverse(node<T> * &nd, size_t key);
    public:
      ConcurrentBinMap();
      ~ConcurrentBinMap();
      
      /*  Stores a given key and a dynamically allocated copy of a value in the map, overiding any existing value */
      void insert(const size_t &key, const T &data);
      /*  Returns the pointer of atomic pointer to the value stored by the key, i.e. std::atomic<value *>*. 
          This pointer is nullptr if no value had been inserted, and can be used to subsequently set or change an inserted value.
          This is equivalent to an insert without setting a value */
      std::atomic<T *> * query(const size_t &key);
      /*  Gets a value that is guarenteed to already be in the map
          This fast but unsafe if value has not already been inserted */
      T& get(const size_t &key);
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
    ConcurrentBinMap<T>::safe_traverse(nd, key);
    T * new_value = new T(data);
    // Will overwrite any existing data assignment
    delete nd->item;
    nd->item.store(new_value, std::memory_order_relaxed);

  }

  template<typename T>
  std::atomic<T *> * ConcurrentBinMap<T>::query(const size_t &key) {
    node<T> * nd = root;
    ConcurrentBinMap<T>::safe_traverse(nd, key);

    // Returns reference of node's pointer to T
    return &(nd->item);

  }

  
  template<typename T>
  T& ConcurrentBinMap<T>::get(const size_t &key) {
    node<T> * nd = root;
    ConcurrentBinMap<T>::quick_traverse(nd, key);

    // Return T
    T * value = nd->item.load(std::memory_order_relaxed);
    return *value;
  }
  
  template<typename T>
  void ConcurrentBinMap<T>::safe_traverse(node<T> * &nd, size_t key) {

    node<T> * new_node;
    node<T> * expected_node;
    size_t idx;

    while (key) {
      // Get the last EXP bits
      idx = key & (BINMAP_WIDTH-1);
        
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

      key = key >> BINMAP_EXP;
      
    }
  }

  template<typename T>
  void ConcurrentBinMap<T>::quick_traverse(node<T> * &nd, size_t key) {

    size_t idx;
    while (key) {
      // Get the last EXP bits
      idx = key & (BINMAP_WIDTH-1);
      nd = nd->next[idx].load(std::memory_order_relaxed);
      key = key >> BINMAP_EXP;
    }
    
  }




}