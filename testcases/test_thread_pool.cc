#include "../src/comm/log.h"
#include <unistd.h>
#include "../src/net/net_address.h"
#include "../src/thread/thread_pool.h"


void* fun(void* arg) {

  DebugLog << "this is test log 1";

  return nullptr;
}

tinyrpc::Logger* gRpcLogger = nullptr;

int main(int argc, char** argv) {

  gRpcLogger = new tinyrpc::Logger();
  gRpcLogger->init("./", "test_thread_pool", 5*1024*1024);

  tinyrpc::ThreadPool pool(4);
  pool.start();

  for (int i = 1; i < 100; ++i) {
    auto cb = [i]() {
      DebugLog << "this is task[" << i << "]";
    };
    pool.addTask(cb);
  }

  DebugLog << "this is main thread, now sleep 10 s";

  sleep(2); 
  DebugLog << "sleep 10 s end, now to stop";

  pool.stop();

  return 0;
}
