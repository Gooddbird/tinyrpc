#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <atomic>
#include "tinyrpc/coroutine/coroutine.h"
#include "tinyrpc/comm/log.h"

namespace tinyrpc {

// 主协程,主协程的栈就是当前线程的栈
// 每个线程都有一个主协程
static thread_local Coroutine* t_main_coroutine = nullptr;

// 线程当前正在执行的协程
static thread_local Coroutine* t_cur_coroutine = nullptr;

static thread_local int t_coroutine_count = 0;

static thread_local int t_cur_coroutine_id = 0;

static thread_local std::string t_msg_no = "";

int getCoroutineIndex() {
  return t_cur_coroutine_id;
}

std::string getCurrentMsgNO() {
  return t_msg_no;
}

void setCurrentMsgNO(const std::string& msgno) {
  t_msg_no = msgno;
}

void CoFunction(Coroutine* co) {

  if (co!= nullptr) {
    co->setIsInCoFunc(true);

    // 去执行协程回调函数
    co->m_call_back();

    co->setIsInCoFunc(false);
  }

  // 执行完协程回调函数返回后,说明协程生命周期结束,此时需恢复到主协程
  Coroutine::Yield();
}

Coroutine::Coroutine() {
  m_cor_id = t_cur_coroutine_id++;
  t_coroutine_count++;
  memset(&m_coctx, 0, sizeof(m_coctx));
}

Coroutine::Coroutine(int size) : m_stack_size(size) {

  if (t_main_coroutine == nullptr) {
    t_main_coroutine = new Coroutine();
  }
  // assert(t_main_coroutine != nullptr);

  m_stack_sp =  reinterpret_cast<char*>(malloc(m_stack_size));
  if (!m_stack_sp) {
    ErrorLog << "start server error. malloc stack return nullptr";
    Exit(0);
  }
  // assert(m_stack_sp != nullptr);

  m_cor_id = t_cur_coroutine_id++;
  t_coroutine_count++;
  // DebugLog << "coroutine[null callback] created, id[" << m_cor_id << "]";
}

Coroutine::Coroutine(int size, std::function<void()> cb)
  : m_stack_size(size) {

  if (t_main_coroutine == nullptr) {
    t_main_coroutine = new Coroutine();
  }
  // assert(t_main_coroutine != nullptr);

  m_stack_sp =  reinterpret_cast<char*>(malloc(m_stack_size));
  if (!m_stack_sp) {
    ErrorLog << "start server error. malloc stack return nullptr";
    Exit(0);
  }
  // assert(m_stack_sp != nullptr);

  setCallBack(cb);
  m_cor_id = t_cur_coroutine_id++;
  t_coroutine_count++;
  // DebugLog << "coroutine created, id[" << m_cor_id << "]";
}

bool Coroutine::setCallBack(std::function<void()> cb) {

  if (this == t_main_coroutine) {
    ErrorLog << "main coroutine can't set callback";
    return false;
  }
  if (m_is_in_cofunc) {
    ErrorLog << "this coroutine is in CoFunction";
    return false;
  }

  m_call_back = cb;

  // assert(m_stack_sp != nullptr);

  char* top = m_stack_sp + m_stack_size;
  // first set 0 to stack
  // memset(&top, 0, m_stack_size);

  top = reinterpret_cast<char*>((reinterpret_cast<unsigned long>(top)) & -16LL);

  memset(&m_coctx, 0, sizeof(m_coctx));

  m_coctx.regs[kRSP] = top;
  m_coctx.regs[kRBP] = top;
  m_coctx.regs[kRETAddr] = reinterpret_cast<char*>(CoFunction); 
  m_coctx.regs[kRDI] = reinterpret_cast<char*>(this);

  return true;

}

Coroutine::~Coroutine() {
  t_coroutine_count--;

  if (m_stack_sp != nullptr) {
    free(m_stack_sp);
    m_stack_sp = nullptr;
  }
  DebugLog << "coroutine[" << m_cor_id << "] die";
}

Coroutine* Coroutine::GetCurrentCoroutine() {
  if (t_cur_coroutine == nullptr) {
    t_main_coroutine = new Coroutine();
    t_cur_coroutine = t_main_coroutine;
  }
  return t_cur_coroutine;
}

bool Coroutine::IsMainCoroutine() {
  if (t_main_coroutine == nullptr || t_cur_coroutine == t_main_coroutine) {
    return true;
  }
  return false;
}

/********
让出执行权,切换到主协程
********/
void Coroutine::Yield() {
  if (t_main_coroutine == nullptr) {
    ErrorLog << "main coroutine is nullptr";
    return;
  }

  if (t_cur_coroutine == t_main_coroutine) {
    ErrorLog << "current coroutine is main coroutine";
    return;
  }
  Coroutine* co = t_cur_coroutine;
  t_cur_coroutine = t_main_coroutine;
  setCurrentMsgNO("");
  coctx_swap(&(co->m_coctx), &(t_main_coroutine->m_coctx));
  // DebugLog << "swap back";
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
    ErrorLog << "pending coroutine is nullptr";
    return;
  }
  t_cur_coroutine = co;
  setCurrentMsgNO(co->getMsgNo());
  coctx_swap(&(t_main_coroutine->m_coctx), &(co->m_coctx));
  // DebugLog << "swap back";

}

}
