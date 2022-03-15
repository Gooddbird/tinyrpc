#include "../src/log/log.h"
#include "../src/coroutine/coroutine.h"
#include <functional>

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

int main(int argc, char* argv[]) {

  tinyrpc::Coroutine::GetCurrentCoroutine();

  tinyrpc::Coroutine::ptr co = std::make_shared<tinyrpc::Coroutine>(128*1024, &fun);

  DebugLog << "this is main co";
  tinyrpc::Coroutine::Resume(co.get());
  DebugLog << "main co back 1";

  tinyrpc::Coroutine::Resume(co.get());
  DebugLog << "main co back 2";

  tinyrpc::Coroutine::ptr co2 = std::make_shared<tinyrpc::Coroutine>(128*1024);
  co2->setCallBack(&fun2);
  
	tinyrpc::Coroutine::Resume(co2.get());

  co2->setCallBack(&fun3);
	tinyrpc::Coroutine::Resume(co2.get());

  return 0;

}
