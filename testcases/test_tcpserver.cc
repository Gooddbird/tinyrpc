#include "../src/net/tcp/tcp_server.h"
#include "../src/net/net_address.h"
#include "../src/comm/log.h"
#include "../src/comm/config.h"

tinyrpc::Logger* gRpcLogger = nullptr;
tinyrpc::Config* gRpcConfig = nullptr;

int main(int argc, char* argv[]) {


  gRpcLogger = new tinyrpc::Logger();
  gRpcLogger->init("test_tcpserver");
  gRpcConfig = new tinyrpc::Config("../testcases/tinyrpc.xml");
  gRpcConfig->readConf();

  tinyrpc::NetAddress::ptr addr = std::make_shared<tinyrpc::IPAddress>("127.0.0.1", 30000);

  tinyrpc::TcpServer tcp_server(addr);
  tcp_server.start();

  return 0;
}
