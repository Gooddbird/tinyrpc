#include "tcp_server.h"
#include "net_address.h"
#include "log.h"
#include "config.h"

tinyrpc::Logger::ptr gRpcLogger;
tinyrpc::Config::ptr gRpcConfig;

int main(int argc, char* argv[]) {

  gRpcConfig = std::make_shared<tinyrpc::Config>("../testcases/tinyrpc.xml");
  gRpcConfig->readConf();

  gRpcLogger = std::make_shared<tinyrpc::Logger>();
  gRpcLogger->init("test_tcpserver");


  tinyrpc::NetAddress::ptr addr = std::make_shared<tinyrpc::IPAddress>("127.0.0.1", 30000);

  tinyrpc::TcpServer tcp_server(addr);
  tcp_server.start();

  return 0;
}
