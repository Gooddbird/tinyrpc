#include <sys/types.h>
#include <sys/socket.h>
#include <assert.h>
#include <arpa/inet.h>
#include <string.h>
#include "../src/comm/log.h"
#include "../src/coroutine/coroutine.h"
#include "../src/coroutine/coroutine_hook.h"
#include "../src/net/reactor.h"

tinyrpc::Logger* gRpcLogger = nullptr; 

void fun1() {
  DebugLog << "this is cor1";
  int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  assert(listenfd != -1);
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

  gRpcLogger = new tinyrpc::Logger();
  gRpcLogger->init("./", "test_coroutine_hook", 5*1024*1024);
  // tinyrpc::enableHook();
  tinyrpc::Coroutine::GetCurrentCoroutine();
  DebugLog << "this is main co";
  tinyrpc::Coroutine::ptr cor1 = std::make_shared<tinyrpc::Coroutine>(1024 * 128, fun1); 
  
  tinyrpc::Coroutine::Resume(cor1.get());

  tinyrpc::Reactor* reactor = tinyrpc::Reactor::GetReactor();
  assert(reactor != nullptr);

  DebugLog << "begin to loop";
  
  reactor->loop();

  return 0;
}
