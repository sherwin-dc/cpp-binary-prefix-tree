#define BINMAP_EXP 3

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
    T item[BINMAP_WIDTH];

    s_node() : next() {

    }
    ~s_node() {

      for (size_t i=0; i<BINMAP_WIDTH; i++) {
        delete next[i];
      }
    }
  };

  template<typename T>
  class SimpleBinMap {
    private:
      s_node<T> * root;
      void safe_traverse(s_node<T> * &nd, size_t key, size_t &idx);
      void quick_traverse(s_node<T> * &nd, size_t key, size_t &idx);
    public:
      SimpleBinMap();
      ~SimpleBinMap();
      
      /*  Stores a given key and a dynamically allocated copy of a value in the map, overiding any existing value */
      void insert(const size_t &key, const T &data);
      /*  Returns the pointer to the value stored by the key, i.e. value *. 
          This is equivalent to an insert without setting a value */
      T* query(const size_t &key);
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
    size_t idx=0;
    SimpleBinMap<T>::safe_traverse(nd, key, idx);

    // Will overwrite any existing data assignment
    nd->item[idx] = data;

  }

  template<typename T>
  T* SimpleBinMap<T>::query(const size_t &key) {
    s_node<T> * nd = root;
    size_t idx=0;
    SimpleBinMap<T>::safe_traverse(nd, key, idx);

    // Returns pointer to T
    return &(nd->item[idx]);

  }

  
  template<typename T>
  T& SimpleBinMap<T>::get(const size_t &key) {
    s_node<T> * nd = root;
    size_t idx=0;
    SimpleBinMap<T>::quick_traverse(nd, key, idx);

    // Return T
    return nd->item[idx];
  }
  
  template<typename T>
  void SimpleBinMap<T>::safe_traverse(s_node<T> * &nd, size_t key, size_t &idx) {

    while (key) {
      // Get the last EXP bits
      idx = key & (BINMAP_WIDTH-1);
      key = key >> BINMAP_EXP;
      if (key) {
        break;
      }
        
      if (!nd->next[idx]) {
        nd->next[idx] = new s_node<T>;
      }

      nd = nd->next[idx];
      
    }
  }

  template<typename T>
  void SimpleBinMap<T>::quick_traverse(s_node<T> * &nd, size_t key, size_t &idx) {

    while (key) {
      // Get the last EXP bits
      idx = key & (BINMAP_WIDTH-1);
      key = key >> BINMAP_EXP; 
      if(key) {
        break;
      }     
      nd = nd->next[idx];

    }
    
  }




}