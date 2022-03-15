#include <vector>
#include "coroutine_pool.h"
#include "coroutine.h"

namespace tinyrpc {


CoroutinePool::CoroutinePool(int pool_size, int stack_size /*= 1024 * 128*/) : m_pool_size(pool_size) {
  m_index = getCoroutineIndex();

  if (m_index == 0) {
    // skip main coroutine
    m_index = 1;
  }

  m_free_cors.resize(pool_size + m_index);
  m_busy_cors.resize(pool_size + m_index);

  for (int i = m_index; i < pool_size + m_index; ++i) {
    m_free_cors[i] = std::make_shared<Coroutine>(stack_size);
  }

}

CoroutinePool::~CoroutinePool();

Coroutine::ptr CoroutinePool::getCoroutineInstanse() {
  for (int i = m_index; i < pool)
}

void CoroutinePool::returnCoroutine(int cor_id);



}


}


#endif