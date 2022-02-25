#ifndef TINYRPC_NET_FD_EVNET_H
#define TINYRPC_NET_FD_EVNET_H

#include <functional>
#include <memory>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <assert.h>
#include "reactor.h"
#include "../log/log.h"
#include "../coroutine/coroutine.h"
#include "mutex.h"

namespace tinyrpc {

class Reactor;

enum IOEvent {
  READ = EPOLLIN,	
  WRITE = EPOLLOUT,  
  ETModel = EPOLLET,
};

class FdEvent : public std::enable_shared_from_this<FdEvent> {
 public:

  typedef std::shared_ptr<FdEvent> ptr;
  
  FdEvent(tinyrpc::Reactor* reactor, int fd = -1);

  FdEvent(int fd);

  virtual ~FdEvent();

  void handleEvent(int flag);

  void setCallBack(IOEvent flag, std::function<void()> cb);

  std::function<void()> getCallBack(IOEvent flag) const;

  void addListenEvents(IOEvent event);

  void delListenEvents(IOEvent event);

  void updateToReactor();

  void unregisterFromReactor ();

  int getFd() const;

  void setFd(const int fd);

  int getListenEvents() const;

	Reactor* getReactor() const;

  void setReactor(Reactor* r);

  void setNonBlock();
  
  bool isNonBlock();

 public:
	Mutex m_mutex;

 protected:
  int m_fd {-1};
  std::function<void()> m_read_callback;
  std::function<void()> m_write_callback;
  
  int m_listen_events {0};
	int m_current_events {0};

  Reactor* m_reactor;

};


class FdEventContainer {

 public:
  FdEventContainer(int size);

  FdEvent::ptr getFdEvent(int fd); 

 public:
  static FdEventContainer* GetFdContainer();

 private:
  RWMutex m_mutex;
  std::vector<FdEvent::ptr> m_fds;

};

}

#endif
