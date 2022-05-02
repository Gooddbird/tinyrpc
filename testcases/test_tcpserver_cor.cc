#include "../src/net/tcp/tcp_server.h"
#include "../src/net/net_address.h"


tinyrpc::Logger* gRpcLogger = nullptr;

int main(int argc, char* argv[]) {

  gRpcLogger = new tinyrpc::Logger();
  gRpcLogger->init("./", "test_tcpserver_cor", 5*1024*1024);

  tinyrpc::IPAddress::ptr addr = std::make_shared<tinyrpc::IPAddress>("127.0.0.1", 39999);
  
  tinyrpc::TcpServer server(addr);
  server.start();
  return 0;
}
