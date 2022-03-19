#ifndef TINYRPC_NET_TCP_ABSTRACTSLOT_H
#define TINYRPC_NET_TCP_ABSTRACTSLOT_H

#include <memory>
#include <functional>

namespace tinyrpc {


template<class T>
class AbstractSlot {
 public:
  typedef std::shared_ptr<AbstractSlot> ptr;
  typedef std::weak_ptr<T> weakPtr;
  typedef std::shared_ptr<T> sharedPtr;

  AbstractSlot(weakPtr ptr, std::function<void(sharedPtr)> cb) : m_weak_ptr(ptr), m_cb(cb) {

  }
  ~AbstractSlot() {
    sharedPtr ptr = m_weak_ptr.lock();
    if (ptr) {
      m_cb(ptr);
    } 
  }

 private:
  weakPtr m_weak_ptr;
  std::function<void(sharedPtr)> m_cb;

};


}
#endif