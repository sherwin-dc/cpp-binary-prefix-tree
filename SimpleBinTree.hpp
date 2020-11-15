#define BINMAP_EXP 3

#include <atomic>
#include <cstddef>

#ifndef BINMAP_EXP
  #error "Width of Map not defined. Before inclusion or at top of the file, define with '#define BINMAP_EXP n' for a s_node width of 2^n"
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
  struct s_node {
    s_node<T> * next[BINMAP_WIDTH];
    T * item;

    s_node() : next() {
      item = nullptr;
    }
    ~s_node() {
      delete item;
      for (size_t i=0; i<BINMAP_WIDTH; i++) {
        delete next[i];
      }
    }
  };

  template<typename T>
  class SimpleBinMap {
    private:
      s_node<T> * root;
      void safe_traverse(s_node<T> * &nd, size_t key);
      void quick_traverse(s_node<T> * &nd, size_t key);
    public:
      SimpleBinMap();
      ~SimpleBinMap();
      
      /*  Stores a given key and a dynamically allocated copy of a value in the map, overiding any existing value */
      void insert(const size_t &key, const T &data);
      /*  Returns the pointer of atomic pointer to the value stored by the key, i.e. value **. 
          This pointer is nullptr if no value had been inserted, and can be used to subsequently set or change an inserted value.
          This is equivalent to an insert without setting a value */
      T** query(const size_t &key);
      /*  Gets a value that is guarenteed to already be in the map
          This fast but unsafe if value has not already been inserted */
      T& get(const size_t &key);
  };

  template<typename T>
  SimpleBinMap<T>::SimpleBinMap() {
    root = new s_node<T>;
  }

  template<typename T>
  SimpleBinMap<T>::~SimpleBinMap() {
    delete root;
  }

  template<typename T>
  void SimpleBinMap<T>::insert(const size_t &key, const T &data) {

    s_node<T> * nd = root;
    SimpleBinMap<T>::safe_traverse(nd, key);
    T * new_value = new T(data);
    // Will overwrite any existing data assignment
    delete nd->item;
    nd->item =new_value;

  }

  template<typename T>
  T** SimpleBinMap<T>::query(const size_t &key) {
    s_node<T> * nd = root;
    SimpleBinMap<T>::safe_traverse(nd, key);

    // Returns pointer of s_node's pointer to T
    return &(nd->item);

  }

  
  template<typename T>
  T& SimpleBinMap<T>::get(const size_t &key) {
    s_node<T> * nd = root;
    SimpleBinMap<T>::quick_traverse(nd, key);

    // Return T
    T * value = nd->item;
    return *value;
  }
  
  template<typename T>
  void SimpleBinMap<T>::safe_traverse(s_node<T> * &nd, size_t key) {

    size_t idx;

    while (key) {
      // Get the last EXP bits
      idx = key & (BINMAP_WIDTH-1);
        
      if (!nd->next[idx]) {
        nd->next[idx] = new s_node<T>;
      }

      nd = nd->next[idx];
      key = key >> BINMAP_EXP;
      
    }
  }

  template<typename T>
  void SimpleBinMap<T>::quick_traverse(s_node<T> * &nd, size_t key) {

    size_t idx;
    while (key) {
      // Get the last EXP bits
      idx = key & (BINMAP_WIDTH-1);
      nd = nd->next[idx];
      key = key >> BINMAP_EXP;
    }
    
  }




}