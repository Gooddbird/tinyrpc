#ifndef TINYRPC_COROUTINE_COUROUTINE_POOL_H
#define TINYRPC_COROUTINE_COUROUTINE_POOL_H

#include <vector>
#include "coroutine.h"

namespace tinyrpc {

class CoroutinePool {

 public:
  CoroutinePool(int pool_size, int stack_size = 1024 * 128);
  ~CoroutinePool();

  Coroutine::ptr getCoroutineInstanse();

  void returnCoroutine(int cor_id);

 private:
  int m_index {0};
  int m_pool_size {0};
  std::vector<Coroutine::ptr> m_free_cors;    // free cors, can be dispatched
  std::vector<Coroutine::ptr> m_busy_cors;

}


}


#endif