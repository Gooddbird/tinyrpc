#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <functional>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>


#include "reactor.h"
#include "timer.h"
#include "log.h"
#include "coroutine.h"
#include "config.h"

tinyrpc::Reactor reactor;
tinyrpc::Coroutine::ptr cor;

tinyrpc::Logger::ptr gRpcLogger; 
tinyrpc::Config::ptr gRpcConfig;


int listenfd = -1;

void* fun(void* arg) {
  sleep(3);
  // DebugLog << "begin to wakeup";
  
  // reactor.wakeup();
  // DebugLog << "end wakeup";
  int fd = listenfd;

  tinyrpc::FdEvent* fd_event = new tinyrpc::FdEvent(&reactor);

	auto readcb = [fd]() {
		DebugLog << "new clent coming!";
    sockaddr_in cli_addr;
    memset(&cli_addr, 0, sizeof(cli_addr));
    socklen_t cli_len = sizeof(cli_addr);
    int connfd = accept(fd, (sockaddr*)&cli_addr, &cli_len);
    DebugLog << "succ to accept new client, clientfd=[" << connfd << "]";
		
	};

	fd_event->setCallBack(tinyrpc::IOEvent::READ, readcb);
	fd_event->addListenEvents(tinyrpc::IOEvent::READ);

	// reactor.addEvent(fd_event, true);
  while(1);

  return nullptr;
}

void accept_f() {

  auto readco = [] () {
    tinyrpc::FdEvent* fd_event = new tinyrpc::FdEvent(&reactor, listenfd);

    int flag = fcntl(fd_event->getFd(), F_GETFL, 0); 

    fcntl(fd_event->getFd(), F_SETFL, flag | O_NONBLOCK);
    flag = fcntl(fd_event->getFd(), F_GETFL, 0); 
    if (flag & O_NONBLOCK) {
      DebugLog << "succ set o_nonblock";
    }

		tinyrpc::Coroutine *thiscor = tinyrpc::Coroutine::GetCurrentCoroutine();
    auto read_call_back = [thiscor]() {
      DebugLog << "occur read, ready to resume";
      tinyrpc::Coroutine::Resume(thiscor);
    };

    fd_event->setCallBack(tinyrpc::IOEvent::READ, read_call_back);
    fd_event->addListenEvents(tinyrpc::IOEvent::READ);

    // fd_event->updateToReactor();

    tinyrpc::Coroutine::Yield();
    DebugLog << "resume back, to accept";

    sockaddr_in cli_addr;
    socklen_t cli_len;
    int cli_fd = accept(fd_event->getFd(), (sockaddr*)&cli_addr, &cli_len); 
    DebugLog << "success accept fd" << cli_fd;
  };

  tinyrpc::Coroutine::GetCurrentCoroutine();
  cor = std::make_shared<tinyrpc::Coroutine>(128 * 1024, readco);
  tinyrpc::Coroutine::Resume(cor.get());

}

int main(int argc, char* argv[]) {


  gRpcConfig = std::make_shared<tinyrpc::Config>("../testcases/tinyrpc.xml");
  gRpcConfig->readConf();

  gRpcLogger = std::make_shared<tinyrpc::Logger>();
  gRpcLogger->init("test_reactor");

	listenfd = -1;	
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
		ErrorLog << "socket error, exit!, errno=" << errno << ", err=" << strerror(errno);
		return -1;
	}

	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(30005);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if ((bind(listenfd, (sockaddr*)&addr, sizeof(addr))) != 0) {
		ErrorLog << "bind error, exit!, errno=" << errno << ", err=" << strerror(errno);
		return -1;
	}

	if (listen(listenfd, 5) != 0) {

		ErrorLog << "listen error, exit!, errno=" << errno << ", err=" << strerror(errno); 
		return -1;
	}

  // tinyrpc::Coroutine::GetCurrentCoroutine();
  // tinyrpc::Coroutine::ptr co = std::make_shared<tinyrpc::Coroutine>(128*1024, &read_f, nullptr);
  // DebugLog << "listen addr=" << inet_ntoa(addr.sin_addr) << ":" << 30000;
  // tinyrpc::Coroutine::Resume(co.get());

	DebugLog << "begin to loop!";

	
	accept_f();

	reactor.loop();

  // pthread_t t_id;
  // pthread_create(&t_id, nullptr, fun, nullptr);

  // auto timer_fun = []() {
    // DebugLog << "timer trigger ";
  // };
  
  // tinyrpc::TimerEvent::ptr timer_event(new tinyrpc::TimerEvent(2000, true, timer_fun));

  // tinyrpc::Timer::ptr timer = reactor.getTimer();
  // timer->addTimerEvent(timer_event);

	return 0;
}




