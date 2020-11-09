#include <atomic>

namespace Tree {


  template<typename T>
  struct node {
    std::atomic<node *> zero;
    std::atomic<node *> one;
    std::atomic<T *> item;

    node() {
      zero = nullptr;
      one = nullptr;
      item = nullptr;
    }
    ~node() {
      delete zero;
      delete one;
      delete item;
    }
  };

  template<typename T>
  class BinMap {
    private:
      node * root;
      void safe_traverse(node<T> * &nd, size_t key);
      void quick_traverse(node<T> * &nd, size_t key);
    public:
      BinMap();
      ~BinMap();
      /*  Stores a given key and a dynamically allocated copy of a value in the map, overiding any existing value */
      void insert(const size_t &key, const T &data);
      /*  Returns the (reference of) atomic pointer to the value stored by the key. 
          This pointer is nullptr if no value had been inserted, and can be used to subsequently set or change an inserted value.
          This is equivalent to an insert without setting a value */
      std::atomic<T *> & query(const size_t &key);
      // T& operator[] (const size_t &key);
      /*  Gets a value that is guarenteed to already be in the map
          This is faster than operator[] but unsafe if value has not already been inserted */
      T& get(const size_t &key);
  };

  template<typename T>
  BinMap<T>::BinMap() {
    root = new node<T>;
  }

  template<typename T>
  BinMap<T>::~BinMap() {
    delete root;
  }

  template<typename T>
  void BinMap<T>::insert(const size_t &key, const T &data) {

    node<T> * nd = root;
    BinMap<T>::safe_traverse(nd, key);
    T * new_value = new T(data);
    // Will overwrite any existing data assignment
    nd->item.store(new_value, std::memory_order_relaxed);

  }

  template<typename T>
  std::atomic<T *> & BinMap<T>::query(const size_t &key) {
    node<T> * nd = root;
    BinMap<T>::safe_traverse(nd, key);

    // Returns reference of node's pointer to T
    return nd->item;

  }

  // template<typename T>
  // T& BinMap<T>::operator[] (const size_t &key) {
  //   node<T> * nd = root;
  //   BinMap<T>::safe_traverse(nd, key);

  //   // Return reference
  // }
  
  template<typename T>
  T& BinMap<T>::get(const size_t &key) {
    node<T> * nd = root;
    BinMap<T>::quick_traverse(nd, key);

    // Return T
    T * value = nd->item.load(std::memory_order_relaxed);
    return *value;
  }
  
  template<typename T>
  void BinMap<T>::safe_traverse(node<T> * &nd, size_t key) {

    node<T> * new_node;
    node<T> * expected_node;

    while (key) {
      // Check if LSB of key is 1 or 0
      if (key & 1) {
        
        expected_node = nd->one.load(std::memory_order_relaxed);
        if (expected_node == nullptr) {
          new_node = new node<T>;
          if (nd->one.compare_exchange_strong(expected_node, new_node, std::memory_order_relaxed, std::memory_order_relaxed)) {
            nd = new_node;
          } else {
            nd = expected_node;
            delete new_node;
          }

        } else {
          nd = expected_node;
        }


      } else {

        expected_node = nd->zero.load(std::memory_order_relaxed);
        if (expected_node == nullptr) {
          new_node = new node<T>;
          if (nd->zero.compare_exchange_strong(expected_node, new_node, std::memory_order_relaxed, std::memory_order_relaxed)) {
            nd = new_node;
          } else {
            nd = expected_node;
            delete new_node;
          }

        } else {
          nd = expected_node;
        }

      }

      key = key >> 1;
      
    }
  }

  template<typename T>
  void BinMap<T>::quick_traverse(node<T> * &nd, size_t key) {

    while (key) {
      // Check if LSB of key is 1 or 0
      if (key & 1) {
        nd = nd->one.load(std::memory_order_relaxed);
      } else {
        nd = nd->zero.load(std::memory_order_relaxed);
      }
      key = key >> 1;
    }

  }




}