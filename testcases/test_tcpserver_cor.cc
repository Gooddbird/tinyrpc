#include "../src/net/tcp/tcp_server.h"
#include "../src/net/net_address.h"



int main(int argc, char* argv[]) {

  tinyrpc::IPAddress::ptr addr = std::make_shared<tinyrpc::IPAddress>("127.0.0.1", 39999);
  
  tinyrpc::TcpServer server(addr);
  server.start();
  return 0;
}
