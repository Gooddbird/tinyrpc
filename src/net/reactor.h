#ifndef TINYRPC_NET_EVENT_LOOP_H
#define TINYRPC_NET_EVENT_LOOP_H

#include <sys/socket.h>
#include <vector>
#include <atomic>
#include <map>
#include "fd_event.h"
#include "mutex.h"

namespace tinyrpc {

class FdEvent;
class Timer;

typedef std::shared_ptr<FdEvent> FdPtr;
typedef std::shared_ptr<Timer> TimerPtr;

class Reactor {

 public:

  Reactor();

  ~Reactor();

  void addEvent(FdPtr fd_event, bool is_wakeup = true);

  void delEvent(FdPtr fd_event, bool is_wakeup = true);

  void addTask(std::function<void()> task, bool is_wakeup = true);

  void addTask(std::vector<std::function<void()>> task, bool is_wakeup = true);
  
  void wakeup();
  
  void loop();

  void stop();

  Timer* getTimer();
 
 public:
  static Reactor* GetReactor();
  

 private:

  void addWakeupFd();

  bool isLoopThread() const;

  void addEventInLoopThread(FdPtr fd_event);

  void delEventInLoopThread(FdPtr fd_event);
  
 private:
  int m_epfd {-1};
  int m_wake_fd {-1};         // wakeup fd
  bool m_stop_flag {false};
  bool m_is_looping {false};
  pid_t m_tid {0};        // thread id

  MutexLock m_mutex;                    // mutex
  
  std::map<int, FdPtr> m_fds;              // alrady case events
  std::atomic<int> m_fd_size; 

  // fds that wait for operate
  // 1 -- to add to loop
  // 2 -- to del from loop
  std::map<FdPtr, int> m_pending_fds;
  std::vector<std::function<void()>> m_pending_tasks;

  Timer* m_timer;

};


}


#endif
