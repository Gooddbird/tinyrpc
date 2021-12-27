#include "../src/log/log.h"
#include <sys/socket.h>
#include <string.h>
#include <sys/type.h>
#include <arpa/inet.h>


int main(int argc, char* argv[]) {
  
  DebugLog << "main begin";

  int connfd = socket(AF_INET, SOCK_STREAM, 0);

  if (connfd == -1) {
    ErrorLog << "socket error, fd=-1, error=" << strerror(errno);
  }
  sockaddr_in ser_addr;
  ser_addr.sin_family = AF_INET;

  ser_addr.sin_port = htons(30000); 
  inet_pton(AF_INET, "192.168.245.7", &ser_addr.sin_addr.s_addr);

  return 0;
  
}
