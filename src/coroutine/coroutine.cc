#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "coroutine.h"
#include "../log/log.h"

namespace tinyrpc {

// 主协程,主协程的栈就是当前线程的栈
// 每个线程都有一个主协程
static thread_local Coroutine* t_main_coroutine = nullptr;

// 线程当前正在执行的协程
static thread_local Coroutine* t_cur_coroutine = nullptr;

static std::atomic<int> g_coroutine_count = 0;

void CoFunction(Coroutine* co) {
  if (co!= nullptr) {
    co->m_call_back(co->m_arg);
  }
  Coroutine::Yield();
}

Coroutine::Coroutine() {
  m_cor_id = g_coroutine_count++;
  memset(&m_coctx, 0, sizeof(m_coctx));
  t_main_coroutine = this;
  t_cur_coroutine = this;
  DebugLog << "" 
}

Coroutine::Coroutine(int size, std::function<void(void*)> cb, void* arg)
  : m_stack_size(size), m_call_back(cb), m_arg(arg) {
  
  if (t_main_coroutine == nullptr) {
    t_main_coroutine = new Coroutine();
  }
  assert(t_main_coroutine != nullptr);

  // 申请堆起始地址
  char* sp =  reinterpret_cast<char*>(malloc(size));

  // 堆最高地址 
  char* top = sp + size;
  // 字节对齐。 因为要作为栈底,所以地址必须是 8字节的整数倍(64位机一个字为8个字节)

  top = reinterpret_cast<char*>((reinterpret_cast<unsigned long>(top)) & -16LL);

  m_cor_id = g_coroutine_count++;
  memset(&m_coctx, 0, sizeof(m_coctx));

  m_coctx.regs[kRSP] = top;
  m_coctx.regs[kRBP] = top;
  m_coctx.regs[kRETAddr] = reinterpret_cast<char*>(CoFunction); 
  m_coctx.regs[kRDI] = reinterpret_cast<char*>(this);

}

Coroutine::~Coroutine() {
  DebugLog << "coroutine[" << m_cor_id << "] die";
}

Coroutine* Coroutine::GetCurrentCoroutine() {
  if (t_cur_coroutine == nullptr) {
    t_cur_coroutine = new Coroutine();
  }
  return t_cur_coroutine;
}

/********
让出执行权,切换到主协程
********/
void Coroutine::Yield() {

  if (t_cur_coroutine == t_main_coroutine) {
    ErrorLog << "current coroutine is main coroutine";
  }

  if (t_main_coroutine == nullptr) {
    ErrorLog << "main coroutine is nullptr";
    return;
  }

  coctx_swap(&(t_cur_coroutine->m_coctx), &(t_main_coroutine->m_coctx));
}

/********
取得执行权,从主协程切换到目标协程
********/
void Coroutine::Resume(Coroutine* co) {

  if (t_cur_coroutine != t_main_coroutine) {
    ErrorLog << "swap error, current coroutine must be main coroutine";
    return;
  }

  if (t_main_coroutine == nullptr) {
    ErrorLog << "main coroutine is nullptr";
    return;
  }

  if (co == nullptr) {
    ErrorLog << "coroutine is nullptr, can't resume";   
    return;
  }

  t_cur_coroutine = co;

  coctx_swap(&(t_main_coroutine->m_coctx), &(co->m_coctx));

}


}
