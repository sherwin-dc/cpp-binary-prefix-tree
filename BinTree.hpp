#include <mutex>

namespace Tree {

  template<typename T>
  struct data_struct {
    T item;
    std::mutex mtx;

    data_struct(const T &item_to_insert) {
      item = item_to_insert;
    }
  };

  template<typename T>
  struct node {
    node * zero;
    node * one;
    data_struct<T> * dt;
    std::mutex mtx;

    node() {
      zero = nullptr;
      one = nullptr;
      dt = nullptr;
    }
    ~node() {
      if (zero != nullptr) {
        delete zero;
      }
      if (one != nullptr) {
        delete one;
      }
      if (dt != nullptr) {
        delete dt;
      }
    }
  };

  template<typename T>
  class BinMap {
    private:
      node * root;
      void BinMap<T>::RecurInsert(size_t &key, const T &data, const node<T> * nd);
      data_struct<T>& RecurFetch(size_t &key, const node<T> * nd);
    public:
      BinMap();
      ~BinMap();
      void Insert(const size_t &key, const T &data);
      T& operator[] (const size_t &key);
      data_struct<T>& Get(const size_t &key);
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
  void BinMap<T>::Insert(const size_t &key, const T &data) {
    size_t key_copy = key;
    BinMap<T>::RecurInsert(key_copy, data, root);
  }

  template<typename T>
  void BinMap<T>::RecurInsert(size_t &key, const T &data, const node<T> * nd) {

    nd->mtx.lock();
    // Check if key is 0
    if (!key) {

      if (nd->dt == nullptr) {
        // Create the data structure
        nd->dt = new data_struct(data);
      } else {
        nd->dt->mtx.lock();
        nd->dt->item = data;
        nd->dt->mtx.unlock();
      }
      nd->mtx.unlock();

    } else {
      // check if the LSB of key is 1 or 0

      if (key & 1) {

        if (nd->one == nullptr) {
          nd->one = new node();
        }
        nd->mtx.unlock();

        // Remove LSB of key and go to next nd
        key = key >> 1;
        BinMap<T>::RecurInsert(key, data, nd->one);

      } else {
        if (nd->zero == nullptr) {
          nd->zero = new node();
        }
        nd->mtx.unlock();

        // Remove LSB of key and go to next nd
        key = key >> 1;
        BinMap<T>::RecurInsert(key, data, nd->zero);

      }
    }
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

  template<typename T>
  data_struct<T>& BinMap<T>::RecurFetch(size_t &key, const node<T> * nd) {
      // Check if key is 0
      if (!key) {
        return nd->dt;
      } else {
        // check if the LSB of key is 1 or 0

        if (key & 1) {
          // Remove LSB of key and go to next nd
          key = key >> 1;
          return BinMap<T>::RecurFetch(key, nd->one);

        } else {

          // Remove LSB of key and go to next nd
          key = key >> 1;
          return BinMap<T>::RecurFetch(key, nd->zero);

        }
      }
  }



}