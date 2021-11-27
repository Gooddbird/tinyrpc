#ifndef TINYRPC_NET_EVENT_LOOP_H
#define TINYRPC_NET_EVENT_LOOP_H

#include <sys/socket.h>
#include <vector>
#include <atomic>
#include <map>
#include "fd_event.h"
#include "mutex.h"

namespace tinyrpc {

class Reactor {

 public:

  Reactor();

  ~Reactor();

  void addEvent(tinyrpc::FdEvent::ptr fd_event, bool is_wakeup = true);

  void delEvent(tinyrpc::FdEvent::ptr fd_event, bool is_wakeup = true);
  
  void wakeup();
  

 private:
  void loop();

  void stop();

  bool init();

  void addEventInLoop(tinyrpc::FdEvent::ptr fd_event);

  void delEventInLoop(tinyrpc::FdEvent::ptr fd_event);


 private:
  int m_epfd {-1};
  int m_wake_fd {-1};         // wakeup fd
  bool m_stop_flag {false};
  pthread_t m_tid {0};        // thread id

  MutexLock m_mutex;                    // mutex
  
  std::map<int, tinyrpc::FdEvent::ptr> m_fds;              // alrady case events
  std::atomic<int> m_fd_size; 

  // fds that wait for operate
  // 1 -- to add to loop
  // 2 -- to del from loop
  std::map<tinyrpc::FdEvent::ptr, int> m_pending_fds;
  std::vector<std::function<void()>> m_pending_tasks;

};


}


#endif
