#ifndef TINYRPC_COROUTINE_COROUTINE_H
#define TINYRPC_COROUTINE_COROUTINE_H

#include <memory>
#include <functional>
#include "tinyrpc/coroutine/coctx.h"
#include "tinyrpc/comm/run_time.h"

namespace tinyrpc {

int getCoroutineIndex();

RunTime* getCurrentRunTime();

void setCurrentRunTime(RunTime* v);

class Coroutine {

 public:
  typedef std::shared_ptr<Coroutine> ptr;

 private:

  Coroutine();

 public:

  Coroutine(int size, char* stack_ptr);

  Coroutine(int size, char* stack_ptr, std::function<void()> cb);

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

  RunTime* getRunTime() {
    return &m_run_time; 
  }

  void setMsgNo(const std::string& msg_no) {
    m_msg_no = msg_no;
  }

  void setIndex(int index) {
    m_index = index;
  }

  int getIndex() {
    return m_index;
  }

  char* getStackPtr() {
    return m_stack_sp;
  }

  int getStackSize() {
    return m_stack_size;
  }

  void setCanResume(bool v) {
    m_can_resume = v;
  }

 public:
  static void Yield();

  static void Resume(Coroutine* cor);

  static Coroutine* GetCurrentCoroutine();

  static Coroutine* GetMainCoroutine();

  static bool IsMainCoroutine();

  // static void SetCoroutineSwapFlag(bool value);

  // static bool GetCoroutineSwapFlag();

 private:
  int m_cor_id {0};        // coroutine' id
  coctx m_coctx;           // coroutine regs
  int m_stack_size {0};         // size of stack memory space
  char* m_stack_sp {NULL};      // coroutine's stack memory space, you can malloc or mmap get some mermory to init this value
  bool m_is_in_cofunc {false};  // true when call CoFunction, false when CoFunction finished
  std::string m_msg_no;
  RunTime m_run_time;

  bool m_can_resume {true};

  int m_index {-1};             // index in coroutine pool


 public:

  std::function<void()> m_call_back;

};

}


#endif
