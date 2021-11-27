#ifndef TINYRPC_MUTEX_H
#define TINYRPC_MUTEX_H

#include <pthread.h>
#include <memory>

namespace tinyrpc {


class MutexLock {

public:
  MutexLock() {
    pthread_mutex_init(&m_mutex, nullptr);
  }

  ~MutexLock() {
    pthread_mutex_destroy(&m_mutex);
  }

  void lock() {
    if (!m_is_lock) {
      pthread_mutex_lock(&m_mutex);
      m_is_lock = true;
    }
  }

  void unlock() {
    if (m_is_lock) {
      pthread_mutex_unlock(&m_mutex);
      m_is_lock = false;
    }
  }

private:
  pthread_mutex_t m_mutex;
  bool m_is_lock {false};
};


class MutexLockGuard {
 public:
  explicit MutexLockGuard(MutexLock& mutex) : m_mutex(mutex) {
    m_mutex.lock();
    m_is_lock = true;
  }

  void unlock() {

  }

  ~MutexLockGuard() {
    m_mutex.unlock();
  }

 private:
  MutexLock& m_mutex;
  bol m_is_lock;
};
 

}


#endif
