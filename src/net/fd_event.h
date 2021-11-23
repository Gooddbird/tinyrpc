#ifndef TINYRPC_NET_FD_EVNET_H
#define TINYRPC_NET_FD_EVNET_H

#include <functional>
#include <memory>
#include <sys/socket.h>
#include "reactor.h"
#include "../log/log.h"

namespace tinyrpc {

enum IOEvent {
  READ = EPOLLIN,
  WRITE = EPOLLOUT,  
  ETModel = EPOLLET,
};

class FdEvent {
 public:
  
  typedef std::shared_ptr<FdEvent> ptr;

  FdEvent(int fd) : m_fd(fd) {
    if (m_fd == -1) {
      LOG << "bad socketfd" << std::endl;
    }
  }
  virtual ~FdEvent() {}

  void handleEvent(int flag) {
    if (flag == READ) {
      m_read_callback();
    } else if (flag == WRITE) {
      m_write_callback();
    } else {
      LOG << "error flag" << std::endl;   
    }

  }
  void setCallBack(IOEvent flag, std::function<void()> cb) {
    if (flag == READ) {
      m_read_callback = cb;
    } else if (flag == WRITE) {
      m_write_callback = cb;
    } else {
      LOG << "error flag" << std::endl;
    }
  }

  void setListenEvents(int flag) {
    m_listen_events = flag;
  }

 private:
  int m_fd {-1};
  std::function<void()> m_read_callback;
  std::function<void()> m_write_callback;

  int m_listen_events;

};

}


#endif
