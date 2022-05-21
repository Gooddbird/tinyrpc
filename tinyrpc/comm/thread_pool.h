#ifndef TINYRPC_THREAD_THRFEADPOOL_H
#define TINYRPC_THREAD_THRFEADPOOL_H

#include <pthread.h>
#include <queue>
#include <functional>
#include "tinyrpc/net/mutex.h"

namespace tinyrpc {

class ThreadPool {
 public:
  ThreadPool(int size);

  ~ThreadPool();

  void start();

  void stop();

  void addTask(std::function<void()> cb);

 private:
  static void* MainFunction(void* ptr);


 public:
  int m_size {0};
  std::vector<pthread_t> m_threads;
  std::queue<std::function<void()>> m_tasks;

  Mutex m_mutex;
  pthread_cond_t m_condition;
  bool m_is_stop {false};
};

}



#endif