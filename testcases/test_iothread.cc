
#include <unistd.h>
#include "../src/net/tcp/io_thread.h"
#include "../src/comm/config.h"



tinyrpc::Logger* gRpcLogger = nullptr; 
tinyrpc::Config* gRpcConfig = nullptr;

int main(int argc, char* argv[]) {

  gRpcLogger = new tinyrpc::Logger();
  gRpcLogger->init("test_iothread");
    gRpcConfig = new tinyrpc::Config("../testcases/tinyrpc.xml");
  gRpcConfig->readConf();

  tinyrpc::IOThread thread;

  sleep(2);

  return 0;
};
