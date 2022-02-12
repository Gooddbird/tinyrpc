#ifndef TINYRPC_COROUTINE_COROUTINE_H
#define TINYRPC_COROUTINE_COROUTINE_H

#include <memory>
#include <functional>
#include "coctx.h"

namespace tinyrpc {


class Coroutine {

 public:
  typedef std::shared_ptr<Coroutine> ptr;

 private:

  Coroutine();

 public:

  Coroutine(int size, std::function<void(void*)> cb, void* arg);

  ~Coroutine();

 public:
  static void Yield();

  static void Resume(Coroutine* cor);

  static Coroutine* GetCurrentCoroutine();

 private:
  int m_cor_id;       // 协程id
  coctx m_coctx;      // 协程寄存器上下文
  int m_stack_size;   // 协程申请堆空间的栈大小,单位: 字节

 public:

  std::function<void(void*)> m_call_back;   // 协程回调函数
  void* m_arg;        // 回调函数参数


};

}


#endif
