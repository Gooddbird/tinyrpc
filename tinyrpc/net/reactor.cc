#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <assert.h>
#include <string.h>
#include <algorithm>
#include "tinyrpc/comm/log.h"
#include "reactor.h"
#include "mutex.h"
#include "fd_event.h"
#include "timer.h"
#include "tinyrpc/coroutine/coroutine.h"
#include "tinyrpc/coroutine/coroutine_hook.h"


extern read_fun_ptr_t g_sys_read_fun;  // sys read func
extern write_fun_ptr_t g_sys_write_fun;  // sys write func

namespace tinyrpc {

static thread_local Reactor* t_reactor_ptr = nullptr;

static thread_local int t_max_epoll_timeout = 10000;     // ms

static CoroutineTaskQueue* t_couroutine_task_queue = nullptr;


Reactor::Reactor() {
  
  // one thread can't create more than one reactor object!!
  // assert(t_reactor_ptr == nullptr);
	if (t_reactor_ptr != nullptr) {
		ErrorLog << "this thread has already create a reactor";
		Exit(0);
	}

  m_tid = gettid();

  DebugLog << "thread[" << m_tid << "] succ create a reactor object";
  t_reactor_ptr = this;

  if((m_epfd = epoll_create(1)) <= 0 ) {
		ErrorLog << "start server error. epoll_create error, sys error=" << strerror(errno);
		Exit(0);
	} else {
		DebugLog << "m_epfd = " << m_epfd;
	}
  // assert(m_epfd > 0);

	if((m_wake_fd = eventfd(0, EFD_NONBLOCK)) <= 0 ) {
		ErrorLog << "start server error. event_fd error, sys error=" << strerror(errno);
		Exit(0);
	}
	DebugLog << "wakefd = " << m_wake_fd;
  // assert(m_wake_fd > 0);	
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
		ErrorLog << "epoo_ctl error, fd[" << fd << "], sys errinfo = " << strerror(errno);
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
		return;
	}
	int op = EPOLL_CTL_DEL;

	if ((epoll_ctl(m_epfd, op, fd, nullptr)) != 0) {
		ErrorLog << "epoo_ctl error, fd[" << fd << "], sys errinfo = " << strerror(errno);
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

  Coroutine* first_coroutine = nullptr;

	while(!m_stop_flag) {
		const int MAX_EVENTS = 10;
		epoll_event re_events[MAX_EVENTS + 1];

    if (first_coroutine) {
      tinyrpc::Coroutine::Resume(first_coroutine);
      first_coroutine = NULL;
    }

    // main reactor need't to resume coroutine in global CoroutineTaskQueue, only io thread do this work
    if (m_reactor_type != MainReactor) {
      FdEvent* ptr = NULL;
      // ptr->setReactor(NULL);
      while(1) {
        ptr = CoroutineTaskQueue::GetCoroutineTaskQueue()->pop();
        if (ptr) {
          ptr->setReactor(this);
          tinyrpc::Coroutine::Resume(ptr->getCoroutine()); 
        } else {
          break;
        }
      }
    }


		// DebugLog << "task";
		// excute tasks
    Mutex::Lock lock(m_mutex);
    std::vector<std::function<void()>> tmp_tasks;
    tmp_tasks.swap(m_pending_tasks);
    lock.unlock();

		for (size_t i = 0; i < tmp_tasks.size(); ++i) {
			// DebugLog << "begin to excute task[" << i << "]";
			if (tmp_tasks[i]) {
				tmp_tasks[i]();
			}
			// DebugLog << "end excute tasks[" << i << "]";
		}
		// DebugLog << "to epoll_wait";
		int rt = epoll_wait(m_epfd, re_events, MAX_EVENTS, t_max_epoll_timeout);

		// DebugLog << "epoll_wait back";

		if (rt < 0) {
			ErrorLog << "epoll_wait error, skip, errno=" << strerror(errno);
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
            int fd = ptr->getFd();

            if ((!(one_event.events & EPOLLIN)) && (!(one_event.events & EPOLLOUT))){
              ErrorLog << "socket [" << fd << "] occur other unknow event:[" << one_event.events << "], need unregister this socket";
              delEventInLoopThread(fd);
            } else {
              // if register coroutine, pengding coroutine to common coroutine_tasks
              if (ptr->getCoroutine()) {
                // the first one coroutine when epoll_wait back, just directly resume by this thread, not add to global CoroutineTaskQueue
                // because every operate CoroutineTaskQueue should add mutex lock
                if (!first_coroutine) {
                  first_coroutine = ptr->getCoroutine();
                  continue;
                }
                if (m_reactor_type == SubReactor) {
                  delEventInLoopThread(fd);
                  ptr->setReactor(NULL);
                  CoroutineTaskQueue::GetCoroutineTaskQueue()->push(ptr);
                } else {
                  // main reactor, just resume this coroutine. it is accept coroutine. and Main Reactor only have this coroutine
                  tinyrpc::Coroutine::Resume(ptr->getCoroutine());
                  if (first_coroutine) {
                    first_coroutine = NULL;
                  }
                }

              } else {
                std::function<void()> read_cb;
                std::function<void()> write_cb;
                read_cb = ptr->getCallBack(READ);
                write_cb = ptr->getCallBack(WRITE);
                // if timer event, direct excute
                if (fd == m_timer_fd) {
                  read_cb();
                  continue;
                }
                if (one_event.events & EPOLLIN) {
                  // DebugLog << "socket [" << fd << "] occur read event";
                  Mutex::Lock lock(m_mutex);
                  m_pending_tasks.push_back(read_cb);						
                }
                if (one_event.events & EPOLLOUT) {
                  // DebugLog << "socket [" << fd << "] occur write event";
                  Mutex::Lock lock(m_mutex);
                  m_pending_tasks.push_back(write_cb);						
                }

              }

            }
          }

				}
				
			}

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
	if (!m_timer) {
		m_timer = new Timer(this);
		m_timer_fd = m_timer->getFd();
	}
	return m_timer;
}

pid_t Reactor::getTid() {
  return m_tid;
}

void Reactor::setReactorType(ReactorType type) {
  m_reactor_type = type;
}


CoroutineTaskQueue* CoroutineTaskQueue::GetCoroutineTaskQueue() {
  if (t_couroutine_task_queue) {
    return t_couroutine_task_queue;
  }
  t_couroutine_task_queue = new CoroutineTaskQueue();
  return t_couroutine_task_queue;

}

void CoroutineTaskQueue::push(FdEvent* cor) {
  Mutex::Lock lock(m_mutex);
  m_task.push(cor);
  lock.unlock();
}

FdEvent* CoroutineTaskQueue::pop() {
  FdEvent* re = nullptr;
  Mutex::Lock lock(m_mutex);
  if (m_task.size() >= 1) {
    re = m_task.front();
    m_task.pop();
  }
  lock.unlock();

  return re;
}

}
