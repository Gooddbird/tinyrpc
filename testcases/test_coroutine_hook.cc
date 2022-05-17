#include <sys/types.h>
#include <sys/socket.h>
#include <assert.h>
#include <arpa/inet.h>
#include <string.h>
#include "log.h"
#include "coroutine.h"
#include "coroutine_hook.h"
#include "reactor.h"
#include "config.h"

tinyrpc::Config::ptr gRpcConfig;

void fun1() {
  DebugLog << "this is cor1";
  int listenfd = socket(AF_INET, SOCK_STREAM, 0);

  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(39999);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  
  if ((bind(listenfd, (sockaddr*)&addr, sizeof(addr))) != 0) {
		ErrorLog << "bind error, exit!, errno=" << errno << ", err=" << strerror(errno);
    return;
  }

	if (listen(listenfd, 5) != 0) {
		ErrorLog << "listen error, exit!, errno=" << errno << ", err=" << strerror(errno); 
		return;
  }
  
  sockaddr_in cli_addr;
  socklen_t cli_len;
  DebugLog << "begin to accept";

  while(1) {

  int connfd = accept(listenfd, (sockaddr*)&cli_addr, (socklen_t*)&cli_len);
  
  DebugLog << "fd : " << connfd << "has acceptd";
  }

}

int main(int argc, char* argv[]) {

  gRpcConfig = std::make_shared<tinyrpc::Config>("../testcases/tinyrpc.xml");
  gRpcConfig->readConf();

  // tinyrpc::enableHook();
  tinyrpc::Coroutine::GetCurrentCoroutine();
  DebugLog << "this is main co";
  tinyrpc::Coroutine::ptr cor1 = std::make_shared<tinyrpc::Coroutine>(1024 * 128, fun1); 
  
  tinyrpc::Coroutine::Resume(cor1.get());

  tinyrpc::Reactor* reactor = tinyrpc::Reactor::GetReactor();

  DebugLog << "begin to loop";
  
  reactor->loop();

  return 0;
}
