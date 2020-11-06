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

  struct node {
    node * zero;
    node * one;
    data_struct * dt;
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

  class BinMap {
    private:
      node * root;
      void BinMap::RecurInsert(size_t &key, const T &data, const node * nd);
      data_struct& RecurFetch(size_t &key, const node * nd);
    public:
      BinMap();
      ~BinMap();
      void Insert(const size_t &key, const T &data);
      T& operator[] (const size_t &key);
      data_struct& Get(const size_t &key);
  }

  BinMap::BinMap() {
    root = new node;
  }

  BinMap::~BinMap() {
    delete root;
  }

  void BinMap::Insert(const size_t &key, const T &data) {
    size_t key_copy = key;
    BinMap::RecurInsert(key_copy, data, root);
  }

  void BinMap::RecurInsert(size_t &key, const T &data, const node * nd) {

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
        BinMap::RecurInsert(key, data, nd->one);

      } else {
        if (nd->zero == nullptr) {
          nd->zero = new node();
        }
        nd->mtx.unlock();

        // Remove LSB of key and go to next nd
        key = key >> 1;
        BinMap::RecurInsert(key, data, nd->zero);

      }
    }
  }

  T& BinMap::operator[] (const size_t &key) {
    size_t key_copy = key;
    data_struct d = RecurFetch(key_copy, root);
    return d.item;
  }
  
  data_struct& BinMap::Get(const size_t &key) {
    size_t key_copy = key;
    data_struct d = RecurFetch(key_copy, root);
    return d;
  }

  data_struct& BinMap::RecurFetch(size_t &key, const node * nd) {
      // Check if key is 0
      if (!key) {
        return nd->dt;
      } else {
        // check if the LSB of key is 1 or 0

        if (key & 1) {
          // Remove LSB of key and go to next nd
          key = key >> 1;
          return BinMap::RecurFetch(key, nd->one);

        } else {

          // Remove LSB of key and go to next nd
          key = key >> 1;
          return BinMap::RecurFetch(key, nd->zero);

        }
      }
  }



}