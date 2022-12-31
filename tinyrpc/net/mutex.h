#ifndef TINYRPC_MUTEX_H
#define TINYRPC_MUTEX_H

#include <pthread.h>
#include <memory>
#include <queue>
#include "tinyrpc/coroutine/coroutine.h"

// this file copy form sylar

namespace tinyrpc {

template <class T>
struct ScopedLockImpl
{
public:
  ScopedLockImpl(T &mutex)
      : m_mutex(mutex)
  {
    m_mutex.lock();
    m_locked = true;
  }

  ~ScopedLockImpl()
  {
    unlock();
  }

  void lock()
  {
    if (!m_locked)
    {
      m_mutex.lock();
      m_locked = true;
    }
  }

  void unlock()
  {
    if (m_locked)
    {
      m_mutex.unlock();
      m_locked = false;
    }
  }

private:
  /// mutex
  T &m_mutex;
  /// 是否已上锁
  bool m_locked;
};

template <class T>
struct ReadScopedLockImpl
{
public:
  ReadScopedLockImpl(T &mutex)
      : m_mutex(mutex)
  {
    m_mutex.rdlock();
    m_locked = true;
  }

  ~ReadScopedLockImpl()
  {
    unlock();
  }

  void lock()
  {
    if (!m_locked)
    {
      m_mutex.rdlock();
      m_locked = true;
    }
  }

  void unlock()
  {
    if (m_locked)
    {
      m_mutex.unlock();
      m_locked = false;
    }
  }

private:
  /// mutex
  T &m_mutex;
  /// 是否已上锁
  bool m_locked;
};

/**
 * @brief 局部写锁模板实现
 */
template <class T>
struct WriteScopedLockImpl
{
public:
  WriteScopedLockImpl(T &mutex)
      : m_mutex(mutex)
  {
    m_mutex.wrlock();
    m_locked = true;
  }

  ~WriteScopedLockImpl()
  {
    unlock();
  }

  void lock()
  {
    if (!m_locked)
    {
      m_mutex.wrlock();
      m_locked = true;
    }
  }

  void unlock()
  {
    if (m_locked)
    {
      m_mutex.unlock();
      m_locked = false;
    }
  }

private:
  T &m_mutex;
  bool m_locked;
};

class Mutex
{
public:
  /// 局部锁
  typedef ScopedLockImpl<Mutex> Lock;

  Mutex()
  {
    pthread_mutex_init(&m_mutex, nullptr);
  }

  ~Mutex()
  {
    pthread_mutex_destroy(&m_mutex);
  }

  void lock()
  {
    pthread_mutex_lock(&m_mutex);
  }

  void unlock()
  {
    pthread_mutex_unlock(&m_mutex);
  }

  pthread_mutex_t *getMutex()
  {
    return &m_mutex;
  }

private:
  /// mutex
  pthread_mutex_t m_mutex;
};

class RWMutex
{
public:
  /// 局部读锁
  typedef ReadScopedLockImpl<RWMutex> ReadLock;

  typedef WriteScopedLockImpl<RWMutex> WriteLock;

  RWMutex()
  {
    pthread_rwlock_init(&m_lock, nullptr);
  }

  ~RWMutex()
  {
    pthread_rwlock_destroy(&m_lock);
  }

  void rdlock()
  {
    pthread_rwlock_rdlock(&m_lock);
  }

  void wrlock()
  {
    pthread_rwlock_wrlock(&m_lock);
  }

  void unlock()
  {
    pthread_rwlock_unlock(&m_lock);
  }

private:
  pthread_rwlock_t m_lock;
};


class CoroutineMutex {
 public:
  typedef ScopedLockImpl<CoroutineMutex> Lock;

  CoroutineMutex();

  ~CoroutineMutex();

  void lock();

  void unlock();
 private:
  bool m_lock {false};
  Mutex m_mutex;
  std::queue<Coroutine*> m_sleep_cors;
};


}
#endif
