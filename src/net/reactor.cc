#include <sys/socket.h>
#include <sys/epoll.h>
#include "../comm/log.h"
#include "reactor.h"
#include "mutex.h"
#include "fd_event.h"


namespace tinyrpc {

Reactor::Reactor() {
	init();
}

Reactor::~Reactor() {
	close(m_epfd);
}

bool Reactor::init() {		
	if((m_epfd = epoll_create(1)) <= 0 ) {
		LOG << "epoll_create error" << std::endl;	
		return false;
	}

	return true;
}


// call by other threads, need lock
void Reactor::addEvent(tinyrpc::FdEvent::ptr fd_event, bool is_wakeup/*=true*/) {
	{
		MutexLockGuard lock(m_mutex);
		m_pending_fds.insert(std::make_pair(fd_event, 1));
	}
	if (is_wakeup) {
		wakeup();
	}
}

// call by other threads, need lock
void Reactor::delEvent(tinyrpc::FdEvent::ptr fd_event, bool is_wakeup/*=true*/) {

	{
		MutexLockGuard lock(m_mutex);
		m_pending_fds.insert(std::make_pair(fd_event, 2));
	}
	if (is_wakeup) {
		wakeup();
	}
}

void Reactor::wakeup() {
	
}

// need't mutex, only this thread call
void Reactor::addEventInLoop(tinyrpc::FdEvent::ptr fd_event) {

	int op = EPOLL_CTL_ADD;
	int tmp_fd = fd_event->getFd();
	if (m_fds.find(tmp_fd) != m_fds.end()) {
		op = EPOLL_CTL_MOD;
	}
	
	epoll_event event;
	event.data.ptr = fd_event.get();
	event.events = fd_event->getListenEvents();

	if (epoll_ctl(m_epfd, op, tmp_fd, &event) != 0) {
		LOG << "epoll_create error" << std::endl;	
		return;
	}
	m_fds.insert(std::make_pair(tmp_fd, fd_event));

	LOG << "add succ, fd[" << tmp_fd << "]" << std::endl;	

}


// need't mutex, only this thread call
void Reactor::delEventInLoop(tinyrpc::FdEvent::ptr fd_event) {

	int tmp_fd = fd_event->getFd();
	if (m_fds.find(tmp_fd) == m_fds.end()) {
		LOG << "fd[" << tmp_fd << "] not in this loop";
	}
	int op = EPOLL_CTL_DEL;

	if ((epoll_ctl(m_epfd, op, tmp_fd, nullptr)) != 0) {
		LOG << "epoo_ctl error, fd[" << tmp_fd << "]" << std::endl; 
	}
	m_fds.erase(m_fds.find(fd_event->getFd()));
	LOG << "del succ, fd[" << tmp_fd << "]" << std::endl;	
	
}

void Reactor::loop() {
	MutexLockGuard lock(m_lock);
	if (m_tid == 0) {
		m_tid = gettid();
		LOG << "this loop now in thread [" << m_tid << "]" << std::endl;
	} else {
		LOG << "error, this loop has already in thread [" << m_tid << "]" << std::endl;
		return;
	}

	while(!m_stop_flag) {

		const int MAX_EVENTS = 10;
		epoll_event re_events[MAX_EVENTS + 1];

		int rt = epoll_wait(m_epfd, re_events, MAX_EVENTS, 10000);

		if (rt < 0) {
			LOG << "epoll_wait error, skip" << std::endl;
		} else {
			for (int i = 0; i < rt; ++i) {
				epoll_event one_event = re_events[i];	
				tinyrpc::FdEvent::ptr ptr;
        ptr.reset((tinyrpc::FdEvent*)one_event.data.ptr);

				int fd = ptr->getFd();
				if (one_event.events & EPOLLIN) {
					LOG << "socket [" << fd << "] occur read event" << std::endl;
					m_pending_tasks.push_back(ptr->getCallBack(READ));						
				}
				if (one_event.events & EPOLLOUT) {
					LOG << "socket [" << fd << "] occur write event" << std::endl;
					m_pending_tasks.push_back(ptr->getCallBack(WRITE));						
				}
			}
			for (size_t i = 0; i < m_pending_tasks.size(); ++i) {
				LOG << "begin to execute tasks" << std::endl;
				m_pending_tasks[i]();
				LOG << "end execute tasks" << std::endl;
			}

			std::map<tinyrpc::FdEvent::ptr, int> tmp;

			{
				MutexLockGuard lock(m_mutex);
				tmp.swap(m_pending_fds);
				m_pending_fds.clear();
			}

			for (auto i = tmp.begin(); i != tmp.end(); ++i) {
				if ((*i).second == 1) {
					addEventInLoop((*i).first);	
				} else {
					delEventInLoop((*i).first);
				}
			}
		}
	}
}

void Reactor::stop() {
  if (!m_stop_flag) {
    m_stop_flag = true;
  }
}

}
