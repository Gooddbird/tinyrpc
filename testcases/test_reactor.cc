#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <functional>
#include <netinet/in.h>
#include <pthread.h>


#include "../src/net/reactor.h"
#include "../src/net/timer.h"
#include "../src/log/log.h"

tinyrpc::Reactor reactor;
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



int main(int argc, char* argv[]) {
	listenfd = -1;	
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
		ErrorLog << "socket error, exit!, errno=" << errno << ", err=" << strerror(errno);
		return -1;
	}
	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(30000);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if ((bind(listenfd, (sockaddr*)&addr, sizeof(addr))) != 0) {
		ErrorLog << "bind error, exit!, errno=" << errno << ", err=" << strerror(errno);
		return -1;
	}
	
	if (listen(listenfd, 5) != 0) {

		ErrorLog << "listen error, exit!, errno=" << errno << ", err=" << strerror(errno); 
		return -1;
	}
	
	// tinyrpc::FdEvent::ptr fd_event(new tinyrpc::FdEvent(listenfd));

	// auto readcb = [listenfd]() {
		// DebugLog << "new clent coming!";
    // sockaddr_in cli_addr;
    // memset(&cli_addr, 0, sizeof(cli_addr));
    // socklen_t cli_len = sizeof(cli_addr);
    // int connfd = accept(listenfd, (sockaddr*)&cli_addr, &cli_len);
    // DebugLog << "succ to accept new client, clientfd=[" << connfd << "]";
		
	// };

	// fd_event->setCallBack(tinyrpc::IOEvent::READ, readcb);
	// fd_event->addListenEvents(tinyrpc::IOEvent::READ);

	// reactor.addEvent(fd_event);
  // DebugLog << "listen addr=" << inet_ntoa(addr.sin_addr) << ":" << 30000;

	DebugLog << "begin to loop!";
  // pthread_t t_id;
  // pthread_create(&t_id, nullptr, fun, nullptr);

  auto timer_fun = []() {
    DebugLog << "timer trigger ";
  };
  
  tinyrpc::TimerEvent::ptr timer_event(new tinyrpc::TimerEvent(2000, true, timer_fun));

  tinyrpc::Timer::ptr timer = reactor.getTimer();
  timer->addTimerEvent(timer_event);
	reactor.loop();

	return 0;
}




