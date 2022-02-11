#include "../src/log/log.h"
#include "../src/coroutine/coroutine.h"
#include <functional>

void fun(void* arg) {
  DebugLog << "this is a sub co";

  tinyrpc::Coroutine::Yield();
  DebugLog << "sub co back";
  // tinyrpc::Coroutine::Yield();
}

int main(int argc, char* argv[]) {

  tinyrpc::Coroutine::GetCurrentCoroutine();

  tinyrpc::Coroutine::ptr co = std::make_shared<tinyrpc::Coroutine>(128*1024, &fun, nullptr);

  DebugLog << "this is main co";
  tinyrpc::Coroutine::Resume(co.get());
  DebugLog << "main co back 1";

  tinyrpc::Coroutine::Resume(co.get());
  DebugLog << "main co back 2";

  return 0;

}
