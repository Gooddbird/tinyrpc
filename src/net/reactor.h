#ifndef TINYRPC_NET_EVENT_LOOP_H
#define TINYRPC_NET_EVENT_LOOP_H

#include <sys/socket.h>
#include <vector.h>
#include "fd_event.h"
#include "mutex.h"

namespace tinyrpc {

class Reactor {

 public:
  Reactor();
  ~Reactor(){}
  
  void addToLoop(tinyrpc::FdEvent::ptr fd_event, int events);

  void delFromLoop(tinyrpc::FdEvent:ptr fd_event);

 private:
  void loop();
  bool init();


 private:
  int m_epfd;
  bool m_stop_flag;
  int m_pending_tasks;
  MutexLock lock;                    // mutex

  std::vector<tinyrpc::FdEvent::ptr> m_fds;        // care events

};


}


#endif
