#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include "tcp_connection.h"
#include "../../coroutine/coroutine_hook.h"

namespace tinyrpc {

TcpConection::TcpConection(tinyrpc::Reactor* reactor, int fd, int buff_size) : m_fd(fd), m_state(Connected) {	
  assert(reactor != nullptr); 
  m_reactor = reactor;
  tinyrpc::enableHook();
  m_fd_event = FdEventContainer::GetFdContainer()->getFdEvent(fd);
  m_fd_event->setReactor(m_reactor);
  initBuffer(buff_size); 

  m_read_cor = std::make_shared<Coroutine>(128 * 1024, std::bind(&TcpConection::MainReadCoFunc, this));
  m_write_cor = std::make_shared<Coroutine>(128 * 1024, std::bind(&TcpConection::MainWriteCoFunc, this));
  m_reactor->addCoroutine(m_read_cor);
  DebugLog << "succ create tcp connection";
}

TcpConection::~TcpConection() {
  DebugLog << "~TcpConection";
}

void TcpConection::initBuffer(int size) {

  // 初始化缓冲区大小
  m_write_buffer = std::make_shared<TcpBuffer>(size);
  m_read_buffer = std::make_shared<TcpBuffer>(size);

}

void TcpConection::asyncRead(std::vector<char>& re, int& size) {
	m_read_buffer->readFromBuffer(re, size);	
}

void TcpConection::asyncWrite(const std::vector<char>& buf) {
	const char* tmp = &buf[0];
	int size = buf.size();
	m_write_buffer->writeToBuffer(tmp, size);
  m_reactor->addCoroutine(m_write_cor);
}

void TcpConection::MainReadCoFunc() {
  while(!m_stop_read) {
    if (m_state != Connected) {
      Coroutine::Yield();
    } 
    if (m_read_buffer->writeAble() == 0) {
      m_read_buffer->resize(2 * m_read_buffer->getSize());
    }

    int read_count = m_read_buffer->writeAble();
    char buf[read_count];

    int rt = read(m_fd, &buf, read_count);		
    if (rt <= 0) {
      ErrorLog << "read empty while occur read event, because of peer close, sys error=" << strerror(errno) << ", now to clear tcp connection";
      clearClient();
    } else {
      DebugLog << "succ read " << rt << " bytes: " << buf << ", count=" << rt;
    }
    m_read_buffer->writeToBuffer(buf, rt);

  }
}

void TcpConection::MainWriteCoFunc() {
  while(!m_stop_write) {
    if (m_state != Connected) {
      Coroutine::Yield();
    } 
    std::vector<char> re;
    m_write_buffer->readFromBuffer(re, m_write_buffer->readAble());
    if (re.size() == 0) {
      DebugLog << "app buffer of fd[" << m_fd << "] no data to write, to yiled this coroutine";
      tinyrpc::Coroutine::Yield();
      continue;
    }

    char* buf = &re[0];	

    int rt = write(m_fd, buf, re.size());
    if (rt == -1) {
      ErrorLog << "write empty, error=" << strerror(errno);
    } else {
      DebugLog << "succ write " << rt << " bytes: " << buf;
    }
    DebugLog << "write end, now yield coroutine";

    Coroutine::Yield();

  }
}


void TcpConection::clearClient() {
  if (m_state == Closed) {
    DebugLog << "this client has closed";
    return;
  }
  // first unregister epoll event
  m_fd_event->unregisterFromReactor(); 
  close(m_fd_event->getFd());
  m_state = Closed;

}

void TcpConection::shutdownConnection() {
  if (m_state == Closed) {
    DebugLog << "this client has closed";
    return;
  }
  m_state = HalfClosing; 
  // call sys shutdown to send FIN
  // wait client done something, client will send FIN
  // and fd occur read event but byte count is 0
  // then will call clearClient to set CLOSED
  shutdown(m_fd_event->getFd(), SHUT_WR);

}


}
