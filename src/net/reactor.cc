#include <sys/socket.h>
#include <sys/epoll.h>
#include "../log/log.h"
#include "reactor.h"
#include "fd_event.h"


namespace tinyrpc {

Reactor::Reactor() {


}

bool Reactor::init() {		
	if((m_epfd = epoll_create(1) <=0 ) {
		LOG << "epoll_create error" << std::endl;	
		return false;
	}
	return true;
}



void addToLoop(tinyrpc::FdEvent::ptr fd_event, int events) {
	int op = EPOLL_CTL_ADD;
	epoll_event event;	
	event.data.fd = fd_event->m_fd;
	event.data.ptr = fd_event;
	event.events = fd_event->m_listen_events;
	// int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);	

	if (epoll_ctl(m_epfd, op, fd_event->m_fd, event) !=0 ) {
		LOG << "epoll_create error" << std::endl;	
		return;
	}
	LOG << "add succ" << std::endl;	

}

void delFromLoop(tinyrpc::FdEvent::ptr fd_event) {

}

void Reactor::loop() {	
	while(!m_stop_flag) {
		
	}
}

}
