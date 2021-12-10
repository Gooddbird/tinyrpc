#ifndef TINYRPC_NET_EVENT_LOOP_H
#define TINYRPC_NET_EVENT_LOOP_H

#include <sys/socket.h>
#include <vector>
#include <atomic>
#include <map>
#include <functional>
#include "fd_event.h"
#include "mutex.h"

namespace tinyrpc {

class FdEvent;
class Timer;

typedef std::shared_ptr<Timer> TimerPtr;

class Reactor {

 public:

  Reactor();

  ~Reactor();

  void addEvent(int fd, epoll_event event, bool is_wakeup = true);

  void delEvent(int fd, bool is_wakeup = true);

  void addTask(std::function<void()> task, bool is_wakeup = true);

  void addTask(std::vector<std::function<void()>> task, bool is_wakeup = true);
  
  void wakeup();
  
  void loop();

  void stop();

  TimerPtr getTimer();
 
 public:
  static Reactor* GetReactor();
  

 private:

  void addWakeupFd();

  bool isLoopThread() const;

  void addEventInLoopThread(int fd, epoll_event event);

  void delEventInLoopThread(int fd);
  
 private:
  int m_epfd {-1};
  int m_wake_fd {-1};         // wakeup fd
  int m_timer_fd {-1};        // timer fd
  bool m_stop_flag {false};
  bool m_is_looping {false};
  bool m_is_init_timer {false};
  pid_t m_tid {0};        // thread id

  MutexLock m_mutex;                    // mutex
  
  std::vector<int> m_fds;              // alrady care events
  std::atomic<int> m_fd_size; 

  // fds that wait for operate
  // 1 -- to add to loop
  // 2 -- to del from loop
  std::map<int, epoll_event> m_pending_add_fds;
  std::vector<int> m_pending_del_fds;
  std::vector<std::function<void()>> m_pending_tasks;

  Timer* m_timer;

};


}


#endif
