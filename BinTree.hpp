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
      if (zero != nullptr) {
        delete zero;
      }
      if (one != nullptr) {
        delete one;
      }
      if (item != nullptr) {
        delete item;
      }
    }
  };

  template<typename T>
  class BinMap {
    private:
      node * root;
      void New_Traverse(node<T> * &nd, size_t key);
    public:
      BinMap();
      ~BinMap();
      /* Stores a given key and a copy of a value in the map, overiding any existing value*/
      void Insert(const size_t &key, const T data);
      T& operator[] (const size_t &key);
      T& Get(const size_t &key);
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
  void BinMap<T>::Insert(const size_t &key, const T data) {

    node<T> * nd = root;
    BinMap<T>::New_Traverse(nd, key);

    // Will overwrite any existing data assignment
    nd->item.store(&data, std::memory_order_relaxed);

  }


  template<typename T>
  T& BinMap<T>::operator[] (const size_t &key) {
    size_t key_copy = key;
    data_struct<T> d = RecurFetch(key_copy, root);
    return d.item;
  }
  
  template<typename T>
  data_struct<T>& BinMap<T>::Get(const size_t &key) {
    size_t key_copy = key;
    data_struct<T> d = RecurFetch(key_copy, root);
    return d;
  }
      
  void BinMap<T>::New_Traverse(node<T> * &nd, size_t key) {

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




}