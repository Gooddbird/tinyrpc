#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <functional>


#include "../src/net/reactor.h"
#include "../src/log/log.h"



int main(int argc, char* argv[]) {
	int listenfd = -1;	
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
	
	tinyrpc::FdEvent::ptr fd_event(new tinyrpc::FdEvent(listenfd));

	auto readcb = []() {
		DebugLog << "read call back!";
		
	};

	fd_event->setCallBack(tinyrpc::IOEvent::READ, readcb);
	fd_event->addListenEvents(tinyrpc::IOEvent::READ);

	tinyrpc::Reactor reactor;
	reactor.addEvent(fd_event);
	DebugLog << "begin to loop!";
	reactor.loop();

	return 0;
}




