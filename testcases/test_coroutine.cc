#include "../src/log/log.h"
#include "../src/coroutine/coroutine.h"
#include <functional>

void fun(void* arg) {
  DebugLog << "this is a co";

  tinyrpc::Coroutine::Yield();
}

int main(int argc, char* argv[]) {

  tinyrpc::Coroutine::GetCurrentCoroutine();

  tinyrpc::Coroutine::ptr co = std::make_shared<tinyrpc::Coroutine>(tinyrpc::Coroutine(128*1024, &fun, nullptr));

  tinyrpc::Coroutine::Resume(co.get());

  DebugLog << "this is main co";

  return 0;

}