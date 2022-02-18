#include "../src/log/log.h"
#include "../src/coroutine/coroutine_hook.h"
#include "../src/coroutine/coroutine.h"


void cor1() {
  DebugLog << "this is cor1";
  
}

int main(int argc, char* argv[]) {
  tinyrpc::Coroutine::GetCurrentCoroutine();
  DebugLog << "this is main co";
  tinyrpc::Coroutine::ptr cor1 = std::make_shared<tinyrpc::Coroutine>(1024 * 128, cor1); 
  
  tinyrpc::Resume(cor1);

  return 0;
}
