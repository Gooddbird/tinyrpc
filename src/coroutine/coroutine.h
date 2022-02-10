#ifndef TINYRPC_COROUTINE_COROUTINE_H
#define TINYRPC_COROUTINE_COROUTINE_H

#include <memory>
#include <functional>
#include "coctx.h"

namespace tinyrpc {


class Coroutine {
 public:
  typedef std::shared_ptr<Coroutine> ptr;

 public:

  Coroutine();

  Coroutine(int size, std::function<void(void*)> cb, void* arg);

  ~Coroutine();

 public:
  static void Yield();

  static void Resume(Coroutine* cor);

  static Coroutine* GetCurrentCoroutine();

 public:
  coctx* m_coctx;
  int m_stack_size;

  std::function<void(void*)> m_call_back;
  void* m_arg;


};

}


#endif