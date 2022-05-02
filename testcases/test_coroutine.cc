#include "../src/comm/log.h"
#include "../src/coroutine/coroutine.h"
#include "../src/coroutine/coroutine_pool.h"
#include <functional>


tinyrpc::Logger* gRpcLogger = nullptr; 
void fun() {
  DebugLog << "this is a sub co 11";

  tinyrpc::Coroutine::Yield();
  DebugLog << "sub co back";
  // tinyrpc::Coroutine::Yield();
}

void fun2() {
	DebugLog << "this is another cor 22";
	DebugLog << "cor2 end";
}

void fun3() {
	DebugLog << "this is function 333333333333 ";
  tinyrpc::Coroutine::Yield();
	DebugLog << "function 33333333333 end";
}

void fun4() {
	DebugLog << "this is test coroutine return pool";
  tinyrpc::Coroutine::Yield();
}

int main(int argc, char* argv[]) {

  gRpcLogger = new tinyrpc::Logger();
  gRpcLogger->init("./", "test_coroutine", 5*1024*1024);

  tinyrpc::Coroutine::GetCurrentCoroutine();

  tinyrpc::CoroutinePool* cor_pool = tinyrpc::GetCoroutinePool();

  // tinyrpc::Coroutine::ptr co = std::make_shared<tinyrpc::Coroutine>(128*1024, &fun);

  tinyrpc::Coroutine::ptr co1 = cor_pool->getCoroutineInstanse();
  co1->setCallBack(&fun);

  DebugLog << "this is main co";
  tinyrpc::Coroutine::Resume(co1.get());
  DebugLog << "main co back 1";

  tinyrpc::Coroutine::Resume(co1.get());
  DebugLog << "main co back 2";

  // tinyrpc::Coroutine::ptr co2 = std::make_shared<tinyrpc::Coroutine>(128*1024);

  tinyrpc::Coroutine::ptr co2 = cor_pool->getCoroutineInstanse();
  co2->setCallBack(&fun2);
  
	tinyrpc::Coroutine::Resume(co2.get());

  co2->setCallBack(&fun3);
	tinyrpc::Coroutine::Resume(co2.get());

  cor_pool->returnCoroutine(co1->getCorId());

  tinyrpc::Coroutine::ptr co3 = cor_pool->getCoroutineInstanse();
  co3->setCallBack(&fun4);
  tinyrpc::Coroutine::Resume(co3.get());


  return 0;

}
