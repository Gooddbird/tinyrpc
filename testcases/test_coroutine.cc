#include <google/protobuf/service.h>
#include <iostream>
#include <pthread.h>
#include <iostream>
#include "tinyrpc/comm/log.h"
#include "tinyrpc/coroutine/coroutine_pool.h"
#include "tinyrpc/coroutine/coroutine.h"


tinyrpc::Coroutine::ptr cor;

void fun1() {
  std::cout << "now fitst resume fun1 coroutine by thread 1" << std::endl;
  std::cout << "now begin to yield fun1 coroutine" << std::endl;
  tinyrpc::Coroutine::Yield();
  std::cout << "fun1 coroutine back, now end" << std::endl;

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
  std::cout << "thread 2 begin" << std::endl;
  std::cout << "now begin to resume fun1 coroutine in thread 2" << std::endl;
  tinyrpc::Coroutine::Resume(cor.get());
  std::cout << "now fun1 coroutine back in thread 2" << std::endl;
  std::cout << "thread 2 end" << std::endl;
  return NULL;
}

int main(int argc, char* argv[]) {

  std::cout << "main begin" << std::endl;
  cor = std::make_shared<tinyrpc::Coroutine>(128*1024, fun1);

  thread1_func(NULL);

  pthread_t thread2;
  pthread_create(&thread2, NULL, &thread2_func, NULL);
  pthread_join(thread2, NULL);
  std::cout << "main end" << std::endl;
}
