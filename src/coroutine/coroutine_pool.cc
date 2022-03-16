#include <vector>
#include "coroutine_pool.h"
#include "coroutine.h"

namespace tinyrpc {

static thread_local CoroutinePool* t_coroutine_container_ptr = nullptr; 
static thread_local int t_coroutine_stack_size = 1024 * 108; 
static thread_local int t_coroutine_pool_size = 100; 


CoroutinePool* GetCoroutinePool() {
  if (!t_coroutine_container_ptr) {
    t_coroutine_container_ptr = new CoroutinePool(t_coroutine_pool_size, t_coroutine_stack_size);
  }
  return t_coroutine_container_ptr;
}


CoroutinePool::CoroutinePool(int pool_size, int stack_size /*= 1024 * 128*/) : m_pool_size(pool_size), m_stack_size(stack_size) {
  m_index = getCoroutineIndex();

  if (m_index == 0) {
    // skip main coroutine
    m_index = 1;
  }

  m_free_cors.resize(pool_size + m_index);

  for (int i = m_index; i < pool_size + m_index; ++i) {
    m_free_cors[i] = std::make_pair(std::make_shared<Coroutine>(stack_size), false);
  }

}

CoroutinePool::~CoroutinePool() {

}

Coroutine::ptr CoroutinePool::getCoroutineInstanse() {

  for (int i = m_index; i < m_pool_size; ++i) {
    if (!m_free_cors[i].first->getIsInCoFunc() && !m_free_cors[i].second) {
      m_free_cors[i].second = true;
      return m_free_cors[i].first;
    }
  }
  int newsize = (int)(1.5 * m_pool_size);
  for (int i = 0; i < newsize - m_pool_size; ++i) {
    m_free_cors.push_back(m_free_cors[i] = std::make_pair(std::make_shared<Coroutine>(m_stack_size), false));
  }
  int tmp = m_pool_size;
  m_pool_size = newsize;
  return m_free_cors[tmp].first;

}

void CoroutinePool::returnCoroutine(int cor_id) {
  m_free_cors[cor_id].second = false;
}



}