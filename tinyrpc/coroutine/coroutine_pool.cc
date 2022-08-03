#include <vector>
#include <sys/mman.h>
#include "tinyrpc/comm/config.h"
#include "tinyrpc/comm/log.h"
#include "tinyrpc/coroutine/coroutine_pool.h"
#include "tinyrpc/coroutine/coroutine.h"
#include "tinyrpc/net/mutex.h"


namespace tinyrpc {

extern tinyrpc::Config::ptr gRpcConfig;

static CoroutinePool* t_coroutine_container_ptr = nullptr; 

CoroutinePool* GetCoroutinePool() {
  if (!t_coroutine_container_ptr) {
    t_coroutine_container_ptr = new CoroutinePool(gRpcConfig->m_cor_pool_size, gRpcConfig->m_cor_stack_size);
  }
  return t_coroutine_container_ptr;
}


CoroutinePool::CoroutinePool(int pool_size, int stack_size /*= 1024 * 128 B*/) : m_pool_size(pool_size), m_stack_size(stack_size) {
  // set main coroutine first
  Coroutine::GetCurrentCoroutine();
  
  int total = pool_size * stack_size;
  m_memory_pool = (char*)mmap(NULL, total, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  printf("we map some memory of %d byte --------------- \n", total);
  if (m_memory_pool == (void*)-1) {
    printf("Start TinyRPC failed, faild to mmap get coroutine memory size, errno=%d, err= %s\n", errno, strerror(errno));
    Exit(0);
  }
  char* tmp = m_memory_pool;

  for (int i = 0; i < pool_size; ++i) {
    Coroutine::ptr cor = std::make_shared<Coroutine>(stack_size, tmp);
    cor->setIndex(i);
    m_free_cors.push_back(std::make_pair(cor, false));
    tmp += m_stack_size;
  }

}

CoroutinePool::~CoroutinePool() {

}

Coroutine::ptr CoroutinePool::getCoroutineInstanse() {

  // from 0 to find first free coroutine which: 1. it.second = false, 2. getIsInCoFunc() is false
  // try our best to reuse used corroutine, and try our best not to choose unused coroutine
  // beacuse used couroutine which used has already write bytes into physical memory, 
  // but unused coroutine no physical memory yet. we just call mmap get virtual address, but not write yet. 
  // so linux will alloc physical when we realy write, that casuse page fault interrupt

  RWMutex::ReadLock rlock(m_mutex);
  for (int i = 0; i < m_pool_size; ++i) {
    if (!m_free_cors[i].first->getIsInCoFunc() && !m_free_cors[i].second) {
      m_free_cors[i].second = true;
      Coroutine::ptr cor = m_free_cors[i].first;
      rlock.unlock();
      return cor;
    }
  }
  rlock.unlock();
  int newsize = m_pool_size;

  int t = newsize * m_stack_size; 
  char* s = (char*)mmap(NULL, t, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  printf("we re map some memory of %d byte --------------- \n", t);
  if (s == (void*)-1) {
    ErrorLog << "Start TinyRPC failed, faild to mmap get coroutine memory size\n";
    return nullptr;
  }
  char* s1 = s;
  int tmp = m_pool_size;
  
  RWMutex::WriteLock wlock(m_mutex);
  for (int i = m_pool_size; i < newsize + m_pool_size; ++i) {
    Coroutine::ptr cor = std::make_shared<Coroutine>(m_stack_size, s1);
    cor->setIndex(i);
    m_free_cors.push_back(std::make_pair(cor, false));
    s1 += m_stack_size;
  }
  m_free_cors[tmp].second = true;
  Coroutine::ptr cor = m_free_cors[tmp].first;
  wlock.unlock();

  m_pool_size += newsize;
  return cor;
}

void CoroutinePool::returnCoroutine(Coroutine::ptr cor) {
  int i = cor->getIndex();
  if (i < m_pool_size) {
    m_free_cors[i].second = false;
  }
}



}