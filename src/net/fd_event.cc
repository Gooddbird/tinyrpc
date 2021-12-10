#include "fd_event.h"

namespace tinyrpc {

FdEvent::FdEvent(tinyrpc::Reactor* reactor) : m_reactor(reactor) {
    if (reactor == nullptr) {
      ErrorLog << "create reactor first";
    }
    assert(reactor != nullptr);

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
  DebugLog << "add succ";
}

void FdEvent::delListenEvents(IOEvent event) {
  if (m_listen_events & event) {

    DebugLog << "delete succ";
    m_listen_events &= ~event;
    return;
  }
  DebugLog << "this event not exist, skip";

}

void FdEvent::updateToReactor() {

  epoll_event event;
  event.events = m_listen_events;
  event.data.ptr = this;

  m_reactor->addEvent(m_fd, event);
}

void FdEvent::unregisterFromReactor () {
  m_reactor->delEvent(m_fd);
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


}
