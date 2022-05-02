
#include <unistd.h>
#include "../src/net/tcp/io_thread.h"



tinyrpc::Logger* gRpcLogger = nullptr; 

int main(int argc, char* argv[]) {

  gRpcLogger = new tinyrpc::Logger();
  gRpcLogger->init("./", "test_iothread", 5*1024*1024);

  tinyrpc::IOThread thread;

  sleep(2);

  return 0;
};
