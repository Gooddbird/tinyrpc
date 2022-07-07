#include <vector>
#include <sys/mman.h>
#include "tinyrpc/comm/config.h"
#include "tinyrpc/comm/log.h"
#include "tinyrpc/coroutine/coroutine_pool.h"
#include "tinyrpc/coroutine/coroutine.h"


namespace tinyrpc {

extern tinyrpc::Config::ptr gRpcConfig;

static thread_local CoroutinePool* t_coroutine_container_ptr = nullptr; 

CoroutinePool* GetCoroutinePool() {
  if (!t_coroutine_container_ptr) {
    t_coroutine_container_ptr = new CoroutinePool(gRpcConfig->m_cor_pool_size, gRpcConfig->m_cor_stack_size);
  }
  return t_coroutine_container_ptr;
}


CoroutinePool::CoroutinePool(int pool_size, int stack_size /*= 1024 * 128*/) : m_pool_size(pool_size), m_stack_size(stack_size) {
  int total = pool_size * stack_size;
  m_memory_pool = (char*)mmap(NULL, total, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  if (m_memory_pool == (void*)-1) {
    printf("Start TinyRPC failed, faild to mmap get coroutine memory size, errno=%d, err= %s\n", errno, strerror(errno));
    Exit(0);
  }
  char* tmp = m_memory_pool;
  m_index = getCoroutineIndex();

  if (m_index == 0) {
    // skip main coroutine
    m_index = 1;
  }

  m_free_cors.resize(pool_size + m_index);

  for (int i = m_index; i < pool_size + m_index; ++i) {
    m_free_cors[i] = std::make_pair(std::make_shared<Coroutine>(stack_size, tmp), false);
    tmp += m_stack_size;
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

  int t = newsize * m_stack_size; 
  char* s = (char*)mmap(NULL, t, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  if (s == (void*)-1) {
    ErrorLog << "Start TinyRPC failed, faild to mmap get coroutine memory size\n";
    return nullptr;
  }
  char* s1 = s;
  for (int i = 0; i < newsize - m_pool_size; ++i) {
    m_free_cors.push_back(m_free_cors[i] = std::make_pair(std::make_shared<Coroutine>(m_stack_size, s1), false));
    s1 += m_stack_size;
  }
  int tmp = m_pool_size;
  m_pool_size = newsize;
  return m_free_cors[tmp].first;

}

void CoroutinePool::returnCoroutine(Coroutine::ptr cor) {
  m_free_cors[cor->getCorId()].second = false;
}



}