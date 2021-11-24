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
	m_fd_size = 32;

	for (int i = 0; i < m_fd_size; ++i) {
		m_fds.push_back(tinyrpc::FdEvent::ptr fd_event(new tinyrpc::FdEvent));
	}

	return true;
}

//void Reactor::addFds(tinyrpc::FdEvent::ptr fd_event) {
//	MutexLockGuard lock(m_mutex);
//	if (fd_event->m_fd < m_fd_size) {
//		m_fds[fd_event->m_fd] = fd_event;
//	} else {
//		m_fd_size *= 2;
//		m_fds.reserve(m_fd_size);
//		m_fds[fd_event->m_fd] = fd_event;
//	}
//}



void addToLoop(tinyrpc::FdEvent::ptr fd_event) {
	int op = EPOLL_CTL_ADD;
	epoll_event event;	
	event.data.fd = fd_event->m_fd;
	event.data.ptr = fd_event.get();
	event.events = fd_event->m_listen_events;

	MutexLockGuard lock(m_mutex);
	if (epoll_ctl(m_epfd, op, fd_event->m_fd, event) != 0) {
		lock.unlock();
		LOG << "epoll_create error" << std::endl;	
		return;
	}

	LOG << "add succ" << std::endl;	

}

void delFromLoop(tinyrpc::FdEvent::ptr fd_event) {

}

void Reactor::loop() {	
	while(!m_stop_flag) {

		const int MAX_EVENTS = 10;
		epoll_event re_events[MAX_EVENTS + 1];
		int rt = epoll_wait(m_epfd, re_events, MAX_EVENTS, 10000);
		if (rt < 0) {
			LOG << "epoll_wait error, skip" << std::endl;
		} else {
			for (int i = 0; i < rt; ++i) {
				epoll_event one_event = epoll_event[i];	
				tinyrpc::FdEvent::ptr ptr.reset((tinyrpc::FdEvent*)one_event.data.ptr);
				int fd = ptr->m_fd;
				if (one_event->events & EPOLLIN) {
					LOG << m_fd << " occur read event" << std::endl;
					m_pending_tasks.push_back(ptr->m_read_callback);						
				}
				if (one_event->events & EPOLLOUT) {
					LOG << m_fd << " occur write event" << std::endl;
					m_pending_tasks.push_back(ptr->m_write_callback);						
				}
			}
			for (int i = 0; i < m_pending_tasks.size(); ++i) {
				LOG << "begin to execute tasks" << std::endl;
				m_pending_tasks[i]();
				LOG << "end execute tasks" << std::endl;
			}
		}
	}
}

}
