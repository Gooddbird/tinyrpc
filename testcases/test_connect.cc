#include "../src/log/log.h"
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>


int main(int argc, char* argv[]) {
  
  DebugLog << "main begin";

  int connfd = socket(AF_INET, SOCK_STREAM, 0);

  if (connfd == -1) {
    ErrorLog << "socket error, fd=-1, error=" << strerror(errno);
  }
  sockaddr_in ser_addr;
  ser_addr.sin_family = AF_INET;

  ser_addr.sin_port = htons(30000); 
  ser_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  // inet_pton(AF_INET, "192.168.245.7", &ser_addr.sin_addr.s_addr);

  int flag = fcntl(connfd, F_GETFL, 0); 

  fcntl(connfd, F_SETFL, flag | O_NONBLOCK);
    // ErrorLog << "fcntl error, error=" << strerror(errno);

  flag = fcntl(connfd, F_GETFL, 0); 
  if (flag & O_NONBLOCK) {
    DebugLog << "succ set o_nonblock";
  }
  DebugLog << "begin to connect";
  while(true) {
    int rt = connect(connfd, reinterpret_cast<sockaddr*>(&ser_addr), sizeof(ser_addr));
    DebugLog << "rt " << rt;
    if (rt == -1) {
      sleep(1);
      DebugLog << "retry connect";
    } else if (rt == 0) {
      DebugLog << "connect succ";
      break;
    } else {
      DebugLog << "unknown rt " << rt;
    }
  }

  while(1) {

    std::string ss;
    std::cin >> ss;
		char buf[4] = {'a', 'b', 'c', 'd'};
    int rt = write(connfd, buf, sizeof(buf));
    DebugLog << "succ write[" << buf << "], write count=" << rt << ", src count=" << sizeof(buf);
  }
  
  return 0;
  
}
