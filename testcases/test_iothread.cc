
#include <unistd.h>
#include "io_thread.h"
#include "config.h"




tinyrpc::Logger::ptr gRpcLogger; 
tinyrpc::Config::ptr gRpcConfig;

int main(int argc, char* argv[]) {

  gRpcConfig = std::make_shared<tinyrpc::Config>("../testcases/tinyrpc.xml");
  gRpcConfig->readConf();

  gRpcLogger = std::make_shared<tinyrpc::Logger>();
  gRpcLogger->init("test_iothread");

  tinyrpc::IOThread thread;

  sleep(2);

  return 0;
};
