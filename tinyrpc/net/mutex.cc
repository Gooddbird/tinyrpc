#include <pthread.h>
#include <memory>
#include "tinyrpc/net/mutex.h"
#include "tinyrpc/net/reactor.h"
#include "tinyrpc/comm/log.h"
#include "tinyrpc/coroutine/coroutine.h"
#include "tinyrpc/coroutine/coroutine_hook.h"

// this file copy form sylar

namespace tinyrpc {


CoroutineMutex::CoroutineMutex() {}

CoroutineMutex::~CoroutineMutex() {
  if (m_lock) {
    unlock();
  }
}

void CoroutineMutex::lock() {

  if (Coroutine::IsMainCoroutine()) {
    ErrorLog << "main coroutine can't use coroutine mutex";
    return;
  }

  Coroutine* cor = Coroutine::GetCurrentCoroutine();

  Mutex::Lock lock(m_mutex);
  if (!m_lock) {
    m_lock = true;
    lock.unlock();
  } else {

    m_sleep_cors.push(cor);
    lock.unlock();

    Coroutine::Yield();
  } 
  DebugLog << "succ get coroutine mutex"; 
}

void CoroutineMutex::unlock() {
  if (Coroutine::IsMainCoroutine()) {
    ErrorLog << "main coroutine can't use coroutine mutex";
    return;
  }

  Mutex::Lock lock(m_mutex);
  if (m_lock) {
    m_lock = false;

    Coroutine* cor = m_sleep_cors.front();
    m_sleep_cors.pop();
    lock.unlock();

    if (cor) {
      // wakeup the first cor in sleep queue
      tinyrpc::Reactor::GetReactor()->addTask([cor]() {
        tinyrpc::Coroutine::Resume(cor);
      }, true);
    }
  }
}


}