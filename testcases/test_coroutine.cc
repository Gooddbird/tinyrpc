#include <google/protobuf/service.h>
#include <iostream>
#include <pthread.h>
#include <iostream>
#include "tinyrpc/comm/log.h"
#include "tinyrpc/coroutine/coroutine_pool.h"
#include "tinyrpc/coroutine/coroutine_hook.h"
#include "tinyrpc/coroutine/coroutine.h"
#include "tinyrpc/net/mutex.h"


tinyrpc::Coroutine::ptr cor;
tinyrpc::Coroutine::ptr cor2;

class Test {

 public:
  tinyrpc::CoroutineMutex m_coroutine_mutex;
  int a = 1;
};
Test test_;

void fun1() {
  std::cout << "cor1 ---- now fitst resume fun1 coroutine by thread 1" << std::endl;
  std::cout << "cor1 ---- now begin to yield fun1 coroutine" << std::endl;

  test_.m_coroutine_mutex.lock();

  std::cout << "cor1 ---- coroutine lock on test_, sleep 5s begin" << std::endl;

  sleep(5);
  std::cout << "cor1 ---- sleep 5s end, now back coroutine lock" << std::endl;

  test_.m_coroutine_mutex.unlock();

  tinyrpc::Coroutine::Yield();
  std::cout << "cor1 ---- fun1 coroutine back, now end" << std::endl;

}

void fun2() {
  std::cout << "cor222 ---- now fitst resume fun1 coroutine by thread 1" << std::endl;
  std::cout << "cor222 ---- now begin to yield fun1 coroutine" << std::endl;

  sleep(2);
  std::cout << "cor222 ---- coroutine2 want to get coroutine lock of test_" << std::endl;
  test_.m_coroutine_mutex.lock();

  std::cout << "cor222 ---- coroutine2 get coroutine lock of test_ succ" << std::endl;

}


void* thread1_func(void*) {
  std::cout << "thread 1 begin" << std::endl;
  std::cout << "now begin to resume fun1 coroutine in thread 1" << std::endl;
  tinyrpc::Coroutine::Resume(cor.get());
  std::cout << "now fun1 coroutine back in thread 1"<< std::endl;
  std::cout << "thread 1 end" << std::endl;
  return NULL;
}

void* thread2_func(void*) {
  tinyrpc::Coroutine::GetCurrentCoroutine();
  std::cout << "thread 2 begin" << std::endl;
  std::cout << "now begin to resume fun1 coroutine in thread 2" << std::endl;
  tinyrpc::Coroutine::Resume(cor2.get());
  std::cout << "now fun1 coroutine back in thread 2" << std::endl;
  std::cout << "thread 2 end" << std::endl;
  return NULL;
}

int main(int argc, char* argv[]) {
  tinyrpc::SetHook(false);
  std::cout << "main begin" << std::endl;
  int stack_size = 128 * 1024;
  char* sp = reinterpret_cast<char*>(malloc(stack_size));
  cor = std::make_shared<tinyrpc::Coroutine>(stack_size, sp, fun1);

  char* sp2 = reinterpret_cast<char*>(malloc(stack_size));
  cor2 = std::make_shared<tinyrpc::Coroutine>(stack_size, sp2, fun2);


  pthread_t thread2;
  pthread_create(&thread2, NULL, &thread2_func, NULL);

  thread1_func(NULL);

  pthread_join(thread2, NULL);

  std::cout << "main end" << std::endl;
}
