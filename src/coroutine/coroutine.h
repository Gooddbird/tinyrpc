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

  Coroutine(int size, std::function<void()> cb);

  ~Coroutine();

  void SetCallBack(std::function<void> cb); 

 public:
  static void Yield();

  static void Resume(Coroutine::ptr cor);

  static Coroutine* GetCurrentCoroutine();

 private:
  int m_cor_id;       // 协程id
  coctx m_coctx;      // 协程寄存器上下文
  int m_stack_size;   // 协程申请堆空间的栈大小,单位: 字节
  char* m_stack_sp;   // 

 public:

  std::function<void()> m_call_back;   // 协程回调函数

};

}


#endif
