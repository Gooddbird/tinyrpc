#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <assert.h>
#include <string.h>
#include <algorithm>
#include "../log/log.h"
#include "reactor.h"
#include "mutex.h"
#include "fd_event.h"
#include "timer.h"
#include "../coroutine/coroutine.h"
#include "../coroutine/coroutine_hook.h"


extern read_fun_ptr_t g_sys_read_fun;  // sys read func
extern write_fun_ptr_t g_sys_write_fun;  // sys write func

namespace tinyrpc {

static thread_local Reactor* t_reactor_ptr = nullptr;

static thread_local int t_max_epoll_timeout = 10000;     // ms


Reactor::Reactor() {
  
  // one thread can't create more than one reactor object!!
  assert(t_reactor_ptr == nullptr);
  m_tid = gettid();

  DebugLog << "thread[" << m_tid << "] succ create a reactor object";
  t_reactor_ptr = this;

  if((m_epfd = epoll_create(1)) <= 0 ) {
		ErrorLog << "epoll_create error";
	} else {
		DebugLog << "m_epfd = " << m_epfd;
	}
  assert(m_epfd > 0);

	if((m_wake_fd = eventfd(0, EFD_NONBLOCK)) <= 0 ) {
		ErrorLog << "eventfd error";
	}
	DebugLog << "wakefd = " << m_wake_fd;
  assert(m_wake_fd > 0);	
	addWakeupFd();

}

Reactor::~Reactor() {
  DebugLog << "~Reactor";
	close(m_epfd);
  if (m_timer != nullptr) {
    delete m_timer;
    m_timer = nullptr;
  }
  t_reactor_ptr = nullptr;
}

Reactor* Reactor::GetReactor() {
  if (t_reactor_ptr == nullptr) {
		DebugLog << "Create new Reactor";
    t_reactor_ptr = new Reactor();
  }
	// DebugLog << "t_reactor_ptr = " << t_reactor_ptr;
  return t_reactor_ptr; 
}

// call by other threads, need lock
void Reactor::addEvent(int fd, epoll_event event, bool is_wakeup/*=true*/) {
  if (fd == -1) {
    ErrorLog << "add error. fd invalid, fd = -1";
    return;
  }
  if (isLoopThread()) {
    addEventInLoopThread(fd, event);
    return;
  }
	{
 		Mutex::Lock lock(m_mutex);
		m_pending_add_fds.insert(std::pair<int, epoll_event>(fd, event));
	}
	if (is_wakeup) {
		wakeup();
	}
}

// call by other threads, need lock
void Reactor::delEvent(int fd, bool is_wakeup/*=true*/) {

  if (fd == -1) {
    ErrorLog << "add error. fd invalid, fd = -1";
    return;
  }

  if (isLoopThread()) {
    delEventInLoopThread(fd);
    return;
  }

	{
    Mutex::Lock lock(m_mutex);
		m_pending_del_fds.push_back(fd);
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
	if(g_sys_write_fun(m_wake_fd, p, 8) != 8) {
		ErrorLog << "write wakeupfd[" << m_wake_fd <<"] error";
	}
}

// m_tid only can be writed in Reactor::Reactor, so it needn't to lock 
bool Reactor::isLoopThread() const {
	if (m_tid == gettid()) {
		// DebugLog << "return true";
		return true;
	}
	// DebugLog << "m_tid = "<< m_tid << ", getttid = " << gettid() <<"return false";
	return false;
}


void Reactor::addWakeupFd() {
	int op = EPOLL_CTL_ADD;
	epoll_event event;
	event.data.fd = m_wake_fd;
	event.events = EPOLLIN;
	if ((epoll_ctl(m_epfd, op, m_wake_fd, &event)) != 0) {
		ErrorLog << "epoo_ctl error, fd[" << m_wake_fd << "], errno=" << errno << ", err=" << strerror(errno) ;
	}
	m_fds.push_back(m_wake_fd);

}

// need't mutex, only this thread call
void Reactor::addEventInLoopThread(int fd, epoll_event event) {

  assert(isLoopThread());

	int op = EPOLL_CTL_ADD;
	bool is_add = true;
	// int tmp_fd = event;
	auto it = find(m_fds.begin(), m_fds.end(), fd);
	if (it != m_fds.end()) {
		is_add = false;
		op = EPOLL_CTL_MOD;
	}
	
	// epoll_event event;
	// event.data.ptr = fd_event.get();
	// event.events = fd_event->getListenEvents();

	if (epoll_ctl(m_epfd, op, fd, &event) != 0) {
		ErrorLog << "epoo_ctl error, fd[" << fd << "]";
		return;
	}
	if (is_add) {
		m_fds.push_back(fd);
	}
	DebugLog << "epoll_ctl add succ, fd[" << fd << "]"; 

}


// need't mutex, only this thread call
void Reactor::delEventInLoopThread(int fd) {

  assert(isLoopThread());

	auto it = find(m_fds.begin(), m_fds.end(), fd);
	if (it == m_fds.end()) {
		DebugLog << "fd[" << fd << "] not in this loop";
	}
	int op = EPOLL_CTL_DEL;

	if ((epoll_ctl(m_epfd, op, fd, nullptr)) != 0) {
		ErrorLog << "epoo_ctl error, fd[" << fd << "]";
	}

	m_fds.erase(it);
	DebugLog << "del succ, fd[" << fd << "]"; 
	
}


void Reactor::loop() {

  assert(isLoopThread());
  if (m_is_looping) {
    // DebugLog << "this reactor is looping!";
    return;
  }
  
  m_is_looping = true;
	m_stop_flag = false;

	while(!m_stop_flag) {
		const int MAX_EVENTS = 10;
		epoll_event re_events[MAX_EVENTS + 1];
		DebugLog << "to epoll_wait";
		int rt = epoll_wait(m_epfd, re_events, MAX_EVENTS, t_max_epoll_timeout);

		// DebugLog << "epoll_waiti back";

		if (rt < 0) {
			ErrorLog << "epoll_wait error, skip";
		} else {
			// DebugLog << "epoll_wait back, rt = " << rt;
			for (int i = 0; i < rt; ++i) {
				epoll_event one_event = re_events[i];	

				if (one_event.data.fd == m_wake_fd && (one_event.events & READ)) {
					// wakeup
					// DebugLog << "epoll wakeup, fd=[" << m_wake_fd << "]";
					char buf[8];
					while(1) {
						if((g_sys_read_fun(m_wake_fd, buf, 8) == -1) && errno == EAGAIN) {
							break;
						}
					}

				} else {
					tinyrpc::FdEvent* ptr = (tinyrpc::FdEvent*)one_event.data.ptr;
          if (ptr != nullptr) {
            int fd;
            std::function<void()> read_cb;
            std::function<void()> write_cb;

            {
              Mutex::Lock lock(ptr->m_mutex);
              fd = ptr->getFd();
              read_cb = ptr->getCallBack(READ);
              write_cb = ptr->getCallBack(WRITE);
            }

            if ((!(one_event.events & EPOLLIN)) && (!(one_event.events & EPOLLOUT))){
              DebugLog << "socket [" << fd << "] occur other unknow event:[" << one_event.events << "], need unregister this socket";
              delEventInLoopThread(fd);
            } else {
              if (one_event.events & EPOLLIN) {
                DebugLog << "socket [" << fd << "] occur read event";
                Mutex::Lock lock(m_mutex);
                m_pending_tasks.push_back(read_cb);						
              }
              if (one_event.events & EPOLLOUT) {
                DebugLog << "socket [" << fd << "] occur write event";
                Mutex::Lock lock(m_mutex);
                m_pending_tasks.push_back(write_cb);						
              }
            }
          }

				}
				
			}
			
			// DebugLog << "task";
			// excute tasks
			for (size_t i = 0; i < m_pending_tasks.size(); ++i) {
				// DebugLog << "begin to excute task[" << i << "]";
				m_pending_tasks[i]();
			  // DebugLog << "end excute tasks[" << i << "]";
			}
      m_pending_tasks.clear();

			std::map<int, epoll_event> tmp_add;
			std::vector<int> tmp_del;

			{
        Mutex::Lock lock(m_mutex);
				tmp_add.swap(m_pending_add_fds);
				m_pending_add_fds.clear();

				tmp_del.swap(m_pending_del_fds);
				m_pending_del_fds.clear();

			}
			for (auto i = tmp_add.begin(); i != tmp_add.end(); ++i) {
				// DebugLog << "fd[" << (*i).first <<"] need to add";
				addEventInLoopThread((*i).first, (*i).second);	
			}
			for (auto i = tmp_del.begin(); i != tmp_del.end(); ++i) {
				// DebugLog << "fd[" << (*i) <<"] need to del";
				delEventInLoopThread((*i));	
			}
		}
	}
  DebugLog << "reactor loop end";
  m_is_looping = false;
}

void Reactor::stop() {
  if (!m_stop_flag && m_is_looping) {
    m_stop_flag = true;
    wakeup();
  }
}


void Reactor::addTask(std::function<void()> task, bool is_wakeup /*=true*/) {

  {
    Mutex::Lock lock(m_mutex);
    m_pending_tasks.push_back(task);
  }
  if (is_wakeup) {
    wakeup();
  }
}

void Reactor::addTask(std::vector<std::function<void()>> task, bool is_wakeup /* =true*/) {

  if (task.size() == 0) {
    return;
  }

  {
    Mutex::Lock lock(m_mutex);
    m_pending_tasks.insert(m_pending_tasks.end(), task.begin(), task.end());
  }
  if (is_wakeup) {
    wakeup();
  }
}

void Reactor::addCoroutine(tinyrpc::Coroutine::ptr cor, bool is_wakeup /*=true*/) {

  auto func = [cor](){
    tinyrpc::Coroutine::Resume(cor.get());
  };
  addTask(func, is_wakeup);
}

Timer* Reactor::getTimer() {
	if (m_is_init_timer) {
		DebugLog << "already init timer!";
	} else {
		m_timer = new Timer(this);
		m_timer_fd = m_timer->getFd();
	}
	return m_timer;
}

pid_t Reactor::getTid() {
  return m_tid;
}

}
