#include "../src/log/log.h"
#include "../src/coroutine/coroutine.h"
#include <functional>

void fun(void* arg) {
  DebugLog << "this is a sub co";

  tinyrpc::Coroutine::Yield();
  DebugLog << "sub co back";
  // tinyrpc::Coroutine::Yield();
}

void fun2(void* arg) {
	DebugLog << "this is another cor ";
  tinyrpc::Coroutine::Yield();
}

int main(int argc, char* argv[]) {

  tinyrpc::Coroutine::GetCurrentCoroutine();

  tinyrpc::Coroutine::ptr co = std::make_shared<tinyrpc::Coroutine>(128*1024, &fun, nullptr);

  DebugLog << "this is main co";
  tinyrpc::Coroutine::Resume(co.get());
  DebugLog << "main co back 1";

  tinyrpc::Coroutine::Resume(co.get());
  DebugLog << "main co back 2";

  tinyrpc::Coroutine::ptr co2 = std::make_shared<tinyrpc::Coroutine>(128*1024, &fun2, nullptr);

	tinyrpc::Coroutine::Resume(co2.get());

  return 0;

}
