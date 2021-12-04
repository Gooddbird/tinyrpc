#ifndef TINYRPC_NET_FD_EVNET_H
#define TINYRPC_NET_FD_EVNET_H

#include <functional>
#include <memory>
#include <sys/socket.h>
#include <sys/epoll.h>
#include "reactor.h"
#include "../log/log.h"
#include "mutex.h"
#include "reactor.h"

namespace tinyrpc {

enum IOEvent {
  READ = EPOLLIN,
  WRITE = EPOLLOUT,  
  ETModel = EPOLLET,
};

// void toStringIO(IOEvent envnt) {
  // swit
// }

class FdEvent {
 public:
  
  typedef std::shared_ptr<FdEvent> ptr;

  FdEvent() {

  }

  FdEvent(int fd) : m_fd(fd) {
    if (m_fd == -1) {
      DebugLog << "bad socketfd";
    }
  }

  virtual ~FdEvent() {}

  void handleEvent(int flag) {
    if (flag == READ) {
      m_read_callback();
    } else if (flag == WRITE) {
      m_write_callback();
    } else {
      ErrorLog << "error flag";
    }

  }
  void setCallBack(IOEvent flag, std::function<void()> cb) {
    if (flag == READ) {
      m_read_callback = cb;
    } else if (flag == WRITE) {
      m_write_callback = cb;
    } else {
      ErrorLog << "error flag";
    }
  }

  std::function<void()> getCallBack(IOEvent flag) const {
    if (flag == READ) {
      return m_read_callback;
    } else if (flag == WRITE) {
      return m_write_callback;
    }
    return nullptr;
  }

  void addListenEvents(IOEvent event) {
    if (m_listen_events & event) {
      DebugLog << "already has this event, skip";
      return;
    }
    m_listen_events |= event;
    DebugLog << "add succ";
  }

  int getFd() const {
    return m_fd;
  }

  void setFd(const int fd) {
    m_fd = fd;
  }

  int getListenEvents() const {
    return m_listen_events; 
  }


 public:
	MutexLock m_mutex;

 private:
  int m_fd {-1};
  std::function<void()> m_read_callback;
  std::function<void()> m_write_callback;

  int m_listen_events {0};

};

}


#endif
