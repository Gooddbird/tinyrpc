#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <assert.h>
#include "../log/log.h"
#include "reactor.h"
#include "mutex.h"
#include "fd_event.h"


namespace tinyrpc {

static thread_local Reactor* t_reactor_ptr = nullptr;

Reactor::Reactor() {
  
  // one thread can't create more than one reactor object!!
  assert(t_reactor_ptr == nullptr);
  m_tid = gettid();

  DebugLog << "thread[" << m_tid << "] succ create a reactor object";
  t_reactor_ptr = this;

  if((m_epfd = epoll_create(1)) <= 0 ) {
		ErrorLog << "epoll_create error";
	}
  assert(m_epfd > 0);

	if((m_wake_fd = eventfd(0, EFD_NONBLOCK)) <= 0 ) {
		ErrorLog << "eventfd error";
	}
  assert(eventfd > 0);	
	addWakeupFd();

}

Reactor::~Reactor() {
	close(m_epfd);
}


Reactor* Reactor::GetReactor() {
  return t_reactor_ptr; 
}



// call by other threads, need lock
void Reactor::addEvent(tinyrpc::FdEvent::ptr fd_event, bool is_wakeup/*=true*/) {
  if (isLoopThread()) {
    addEventInLoopThread(fd_event);
    return;
  }
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

  if (isLoopThread()) {
    delEventInLoopThread(fd_event);
    return;
  }

	{
		MutexLockGuard lock(m_mutex);
		m_pending_fds.insert(std::make_pair(fd_event, 2));
	}
	if (is_wakeup) {
		wakeup();
	}
}

void Reactor::wakeup() {

  if (!m_is_looping) {
    return;
  }

	uint64_t tmp = 1;
	uint64_t* p = &tmp; 
	if(write(m_wake_fd, p, 8) != 8) {
		ErrorLog << "write wakeupfd[" << m_wake_fd <<"] error";
	}
}

// m_tid only can be writed in Reactor::Reactor, so it needn't to lock 
bool Reactor::isLoopThread() const {

	return (m_tid == gettid());
}


void Reactor::addWakeupFd() {
	int op = EPOLL_CTL_ADD;
	epoll_event event;
	event.data.fd = m_wake_fd;
	event.events = EPOLLIN;
	if ((epoll_ctl(m_epfd, op, m_wake_fd, &event)) <= 0) {
		ErrorLog << "epoo_ctl error, fd[" << m_wake_fd << "]";
	}

}

// need't mutex, only this thread call
void Reactor::addEventInLoopThread(tinyrpc::FdEvent::ptr fd_event) {

  assert(isLoopThread());

	int op = EPOLL_CTL_ADD;
	int tmp_fd = fd_event->getFd();
	if (m_fds.find(tmp_fd) != m_fds.end()) {
		op = EPOLL_CTL_MOD;
	}
	
	epoll_event event;
	event.data.ptr = fd_event.get();
	event.events = fd_event->getListenEvents();

	if (epoll_ctl(m_epfd, op, tmp_fd, &event) != 0) {
		ErrorLog << "epoo_ctl error, fd[" << tmp_fd << "]";
		return;
	}
	m_fds.insert(std::make_pair(tmp_fd, fd_event));

	

}


// need't mutex, only this thread call
void Reactor::delEventInLoopThread(tinyrpc::FdEvent::ptr fd_event) {

  assert(isLoopThread());

	int tmp_fd = fd_event->getFd();
	if (m_fds.find(tmp_fd) == m_fds.end()) {
		DebugLog << "fd[" << tmp_fd << "] not in this loop";
	}
	int op = EPOLL_CTL_DEL;

	if ((epoll_ctl(m_epfd, op, tmp_fd, nullptr)) != 0) {
		ErrorLog << "epoo_ctl error, fd[" << tmp_fd << "]";
	}
	m_fds.erase(m_fds.find(fd_event->getFd()));
	DebugLog << "del succ, fd[" << tmp_fd << "]"; 
	
}

void Reactor::loop() {

  assert(isLoopThread());
  
  m_is_looping = true;

	while(!m_stop_flag) {
		const int MAX_EVENTS = 10;
		epoll_event re_events[MAX_EVENTS + 1];

		int rt = epoll_wait(m_epfd, re_events, MAX_EVENTS, 10000);

		if (rt < 0) {
			DebugLog << "epoll_wait error, skip";
		} else {
			for (int i = 0; i < rt; ++i) {
				epoll_event one_event = re_events[i];	

				if (one_event.data.fd == m_wake_fd && (one_event.events & READ)) {
					// wakeup
					char buf[8];
					while(1) {
						if((read(m_wake_fd, buf, 8) == 0) && errno == EAGAIN) {
							break;
						}
					}

				}

				tinyrpc::FdEvent::ptr ptr;
        ptr.reset((tinyrpc::FdEvent*)one_event.data.ptr);

				int fd = ptr->getFd();
				if (one_event.events & EPOLLIN) {
					DebugLog << "socket [" << fd << "] occur read event";
					m_pending_tasks.push_back(ptr->getCallBack(READ));						
				}
				if (one_event.events & EPOLLOUT) {
					DebugLog << "socket [" << fd << "] occur write event";
					m_pending_tasks.push_back(ptr->getCallBack(WRITE));						
				}
			}
			
			// excute tasks
			for (size_t i = 0; i < m_pending_tasks.size(); ++i) {
				DebugLog << "begin to excute tasks";
				m_pending_tasks[i]();
				DebugLog << "end excute tasks";
			}
      m_pending_tasks.clear();

			std::map<tinyrpc::FdEvent::ptr, int> tmp;

			{
				MutexLockGuard lock(m_mutex);
				tmp.swap(m_pending_fds);
				m_pending_fds.clear();
			}

			for (auto i = tmp.begin(); i != tmp.end(); ++i) {
				if ((*i).second == 1) {
					addEventInLoopThread((*i).first);	
				} else {
					delEventInLoopThread((*i).first);
				}
			}
		}
	}
  m_is_looping = false;
}

void Reactor::stop() {
  if (!m_stop_flag && m_is_looping) {
    m_stop_flag = true;
    wakeup();
  }
}

}
