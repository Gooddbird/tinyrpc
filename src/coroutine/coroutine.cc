#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "coroutine.h"
#include "../log/log.h"

namespace tinyrpc {

static thread_local Coroutine* t_main_coroutine = nullptr;

static thread_local Coroutine* t_cur_coroutine = nullptr;

enum {
  kRBP = 6,
  kRDI = 7,
  kRSI = 8,
  kRETAddr = 9,
  kRSP = 13,
};

void Cor_fun(Coroutine* co) {
  if (co!= nullptr) {
    co->m_call_back(co->m_arg);
  }
}

Coroutine::Coroutine() {
  m_coctx = new coctx();
  memset(m_coctx, 0, sizeof(*m_coctx));
  t_main_coroutine = this;
  t_cur_coroutine = this;
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
  // 字节对齐。 因为要作为栈底
  top = reinterpret_cast<char*>((reinterpret_cast<unsigned long>(top)) & -16LL);

  m_coctx = new coctx();

  memset(m_coctx, 0, sizeof(*m_coctx));

  m_coctx->regs[kRSP] = top;
  m_coctx->regs[kRBP] = top;
  m_coctx->regs[kRETAddr] = reinterpret_cast<char*>(Cor_fun); 
  m_coctx->regs[kRDI] = reinterpret_cast<char*>(this);

}

Coroutine::~Coroutine() {


}

Coroutine* Coroutine::GetCurrentCoroutine() {
  if (t_cur_coroutine == nullptr) {
    t_cur_coroutine = new Coroutine();
  }
  return t_cur_coroutine;
}

void Coroutine::Yield() {
  if (t_main_coroutine == nullptr) {
    ErrorLog << "main coroutine is nullptr";
    return;
  }
  coctx_swap(t_cur_coroutine->m_coctx, t_main_coroutine->m_coctx);

}

void Coroutine::Resume(Coroutine* co) {
  if (t_main_coroutine == nullptr) {
    ErrorLog << "main coroutine is nullptr";
    return;
  }
  if (co == nullptr) {
    ErrorLog << "coroutine is nullptr, can't resume";   
    return;
  }
  t_cur_coroutine = co;
  coctx_swap(t_main_coroutine->m_coctx, co->m_coctx);

}


}