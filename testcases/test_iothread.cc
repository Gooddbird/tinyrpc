
#include <unistd.h>
#include "io_thread.h"
#include "config.h"

tinyrpc::Config::ptr gRpcConfig;

int main(int argc, char* argv[]) {

  gRpcConfig = std::make_shared<tinyrpc::Config>("../testcases/tinyrpc.xml");
  gRpcConfig->readConf();

  tinyrpc::IOThread thread;

  sleep(2);

  return 0;
};
