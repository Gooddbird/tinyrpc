#include <fcntl.h>
#include <unistd.h>
#include "fd_event.h"

namespace tinyrpc {

static FdEventContainer* g_FdContainer = nullptr;

FdEvent::FdEvent(tinyrpc::Reactor* reactor, int fd/*=-1*/) : m_fd(fd), m_reactor(reactor) {
    if (reactor == nullptr) {
      ErrorLog << "create reactor first";
    }
    // assert(reactor != nullptr);
}

FdEvent::FdEvent(int fd) : m_fd(fd) {

}

FdEvent::~FdEvent() {}

void FdEvent::handleEvent(int flag) {

  if (flag == READ) {
    m_read_callback();
  } else if (flag == WRITE) {
    m_write_callback();
  } else {
    ErrorLog << "error flag";
  }

}

void FdEvent::setCallBack(IOEvent flag, std::function<void()> cb) {
  if (flag == READ) {
    m_read_callback = cb;
  } else if (flag == WRITE) {
    m_write_callback = cb;
  } else {
    ErrorLog << "error flag";
  }
}

std::function<void()> FdEvent::getCallBack(IOEvent flag) const {
  if (flag == READ) {
    return m_read_callback;
  } else if (flag == WRITE) {
    return m_write_callback;
  }
  return nullptr;
}

void FdEvent::addListenEvents(IOEvent event) {
  if (m_listen_events & event) {
    DebugLog << "already has this event, skip";
    return;
  }
  m_listen_events |= event;
  updateToReactor();
  // DebugLog << "add succ";
}

void FdEvent::delListenEvents(IOEvent event) {
  if (m_listen_events & event) {

    DebugLog << "delete succ";
    m_listen_events &= ~event;
    updateToReactor();
    return;
  }
  DebugLog << "this event not exist, skip";

}

void FdEvent::updateToReactor() {

  epoll_event event;
  event.events = m_listen_events;
  event.data.ptr = this;
  // DebugLog << "reactor = " << m_reactor << "log m_tid =" << m_reactor->getTid();
  if (!m_reactor) {
    m_reactor = tinyrpc::Reactor::GetReactor();
  }

  m_reactor->addEvent(m_fd, event);
}

void FdEvent::unregisterFromReactor () {
  if (!m_reactor) {
    m_reactor = tinyrpc::Reactor::GetReactor();
  }
  m_reactor->delEvent(m_fd);
  m_listen_events = 0;
  m_read_callback = nullptr;
  m_write_callback = nullptr;
}

int FdEvent::getFd() const {
  return m_fd;
}

void FdEvent::setFd(const int fd) {
  m_fd = fd;
}

int FdEvent::getListenEvents() const {
  return m_listen_events; 
}

Reactor* FdEvent::getReactor() const {
  return m_reactor;
}

void FdEvent::setReactor(Reactor* r) {
  m_reactor = r;
}

void FdEvent::setNonBlock() {
  if (m_fd == -1) {
    ErrorLog << "error, fd=-1";
    return;
  }

  int flag = fcntl(m_fd, F_GETFL, 0); 
  if (flag & O_NONBLOCK) {
    DebugLog << "fd already set o_nonblock";
    return;
  }

  fcntl(m_fd, F_SETFL, flag | O_NONBLOCK);
  flag = fcntl(m_fd, F_GETFL, 0); 
  if (flag & O_NONBLOCK) {
    DebugLog << "succ set o_nonblock";
  } else {
    ErrorLog << "set o_nonblock error";
  }

}

bool FdEvent::isNonBlock() {
  if (m_fd == -1) {
    ErrorLog << "error, fd=-1";
    return false;
  }
  int flag = fcntl(m_fd, F_GETFL, 0); 
  return (flag & O_NONBLOCK);

}

void FdEvent::setCoroutine(Coroutine* cor) {
  m_coroutine = cor;
}

void FdEvent::clearCoroutine() {
  m_coroutine = nullptr;
}

Coroutine* FdEvent::getCoroutine() {
  return m_coroutine;
}



FdEvent::ptr FdEventContainer::getFdEvent(int fd) {

  RWMutex::ReadLock rlock(m_mutex);
  if (fd < static_cast<int>(m_fds.size())) {
    tinyrpc::FdEvent::ptr re = m_fds[fd]; 
    rlock.unlock();
    return re;
  }
  rlock.unlock();

  RWMutex::WriteLock wlock(m_mutex);
  int n = (int)(fd * 1.5);
  for (int i = m_fds.size(); i < n; ++i) {
    m_fds.push_back(std::make_shared<FdEvent>(i));
  }
  tinyrpc::FdEvent::ptr re = m_fds[fd]; 
  wlock.unlock();
  return re;

}

FdEventContainer::FdEventContainer(int size) {
  for(int i = 0; i < size; ++i) {
    m_fds.push_back(std::make_shared<FdEvent>(i));
  }

}

FdEventContainer* FdEventContainer::GetFdContainer() {
  if (g_FdContainer == nullptr) {
    g_FdContainer = new FdEventContainer(1000); 
  }
  return g_FdContainer;
}


}
