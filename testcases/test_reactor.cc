#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <functional>


#include "../src/net/reactor.h"
#include "../src/comm/log.h"



int main(int argc, char* argv[]) {
	int listenfd = -1;	
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
		LOG << "socket error, exit!, errno=" << errno << ", err=" << strerror(errno) << std::endl;	
		return -1;
	}
	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(30000);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if ((bind(listenfd, (sockaddr*)&addr, sizeof(addr))) != 0) {
		LOG << "bind error, exit!, errno=" << errno << ", err=" << strerror(errno) << std::endl;	
		return -1;
	}
	
	if (listen(listenfd, 5) != 0) {

		LOG << "listen error, exit!, errno=" << errno << ", err=" << strerror(errno) << std::endl;	
		return -1;
	}
	
	tinyrpc::FdEvent::ptr fd_event(new tinyrpc::FdEvent(listenfd));

	auto readcb = []() {
		LOG << "read call back!" << std::endl;
		
	};

	fd_event->setCallBack(tinyrpc::IOEvent::READ, readcb);
	fd_event->addListenEvents(tinyrpc::IOEvent::READ);

	tinyrpc::Reactor reactor;
	reactor.addEvent(fd_event);
	LOG << "begin to loop!" << std::endl;
	reactor.loop();

	return 0;
}




