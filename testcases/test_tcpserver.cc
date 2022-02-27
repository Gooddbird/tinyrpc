#include "../src/net/tcp/tcp_server.h"
#include "../src/net/net_address.h"
#include "../src/log/log.h"


int main(int argc, char* argv[]) {


  tinyrpc::NetAddress::ptr addr = std::make_shared<tinyrpc::IPAddress>("127.0.0.1", 30000);

  tinyrpc::TcpServer tcp_server(addr);
  tcp_server.start();

  return 0;
}
