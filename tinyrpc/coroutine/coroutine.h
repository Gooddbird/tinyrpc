#ifndef TINYRPC_COROUTINE_COROUTINE_H
#define TINYRPC_COROUTINE_COROUTINE_H

#include <memory>
#include <functional>
#include "tinyrpc/coroutine/coctx.h"

namespace tinyrpc {

int getCoroutineIndex();

std::string getCurrentMsgNO();

void setCurrentMsgNO(const std::string& msgno);

class Coroutine {

 public:
  typedef std::shared_ptr<Coroutine> ptr;

 private:

  Coroutine();

 public:

  Coroutine(int size);

  Coroutine(int size, std::function<void()> cb);

  ~Coroutine();

  bool setCallBack(std::function<void()> cb); 

  int getCorId() const {
    return m_cor_id;
  }

  void setIsInCoFunc(const bool v) {
    m_is_in_cofunc = v;
  }

  bool getIsInCoFunc() const {
    return m_is_in_cofunc;
  }

  std::string getMsgNo() {
    return m_msg_no;
  }

  void setMsgNo(const std::string& msg_no) {
    m_msg_no = msg_no;
  }

 public:
  static void Yield();

  static void Resume(Coroutine* cor);

  static Coroutine* GetCurrentCoroutine();

  static bool IsMainCoroutine();

 private:
  int m_cor_id {0};       // 协程id
  coctx m_coctx;      // 协程寄存器上下文
  int m_stack_size {0};   // 协程申请堆空间的栈大小,单位: 字节
  char* m_stack_sp {nullptr};   // 
  bool m_is_in_cofunc {false};  // 是否开始执行。只要协程进入CoFunction就变为true, CoFunction执行完变为false
  std::string m_msg_no;  // 当前协程正在处理的消息号

 public:

  std::function<void()> m_call_back;   // 协程回调函数

};

}


#endif
