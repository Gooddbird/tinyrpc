#include "../src/comm/log.h"
#include <unistd.h>
#include "../src/net/net_address.h"


void* fun(void* arg) {

  DebugLog << "this is test log 1";

  return nullptr;
}

int main(int argc, char** argv) {
  
  DebugLog << "this is test log 1";
  sleep(2);
  InfoLog << "this is test log 2";
  WarnLog << "this is test log 3";
  ErrorLog << "this is test log 4";
  
  pthread_t tid;
  pthread_create(&tid, nullptr, fun, nullptr);
  pthread_join(tid, nullptr);

  tinyrpc::NetAddress::ptr ip_v4(new tinyrpc::IPAddress("127.0.0.1", 30000));

  DebugLog << "ip address: " << ip_v4->toString();

  std::string path = "./tmp";
  tinyrpc::NetAddress::ptr unix_addr(new tinyrpc::UnixDomainAddress(path));
  DebugLog << "unix address: " << unix_addr->toString();



  return 0;
}
