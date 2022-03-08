#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include "tcp_connection.h"
#include "tcp_server.h"
#include "../tinypb/tinypb_codec.h"
#include "../tinypb/tinypb_data.h"
#include "../../coroutine/coroutine_hook.h"

namespace tinyrpc {

TcpConnection::TcpConnection(tinyrpc::TcpServer* tcp_svr, tinyrpc::Reactor* reactor, int fd, int buff_size)
  : m_fd(fd), m_state(Connected) {	
  assert(reactor != nullptr); 
  m_reactor = reactor;

  assert(tcp_svr!= nullptr); 
  m_tcp_svr = tcp_svr;

  tinyrpc::enableHook();
  m_fd_event = FdEventContainer::GetFdContainer()->getFdEvent(fd);
  m_fd_event->setReactor(m_reactor);
  initBuffer(buff_size); 

  m_read_cor = std::make_shared<Coroutine>(128 * 1024, std::bind(&TcpConnection::MainReadCoFunc, this));
  m_write_cor = std::make_shared<Coroutine>(128 * 1024, std::bind(&TcpConnection::MainWriteCoFunc, this));
  m_reactor->addCoroutine(m_read_cor);
  
  m_codec = std::make_shared<TinyPbCodeC>();

  DebugLog << "succ create tcp connection";
}

TcpConnection::~TcpConnection() {
  DebugLog << "~TcpConnection, fd=" << m_fd;
}

void TcpConnection::initBuffer(int size) {

  // 初始化缓冲区大小
  m_write_buffer = std::make_shared<TcpBuffer>(size);
  m_read_buffer = std::make_shared<TcpBuffer>(size);

}

// void TcpConnection::asyncRead(std::vector<char>& re, int& size) {
// 	m_read_buffer->readFromBuffer(re, size);	
// }

void TcpConnection::asyncWrite() {

  if (m_state != Connected) {
    ErrorLog << "send error! TcpConnection is not Connected";
    return;
  }
  if (m_write_buffer->readAble() == 0) {
    ErrorLog << "send error! no data in write buffer";
    return;
  }
  m_reactor->addCoroutine(m_write_cor);
}

void TcpConnection::MainReadCoFunc() {
  while(!m_stop_read) {
    if (m_state == Closed) {
      break;
    }
    bool read_all = false;
    bool close_flag = false;
    while (!read_all) {
      if (m_read_buffer->writeAble() == 0) {
        m_read_buffer->resize(2 * m_read_buffer->getSize());
      }
      int read_count = m_read_buffer->writeAble();
      int write_index = m_read_buffer->writeIndex();

      int rt = read(m_fd, &(m_read_buffer->getBufferVector()[write_index]), read_count);		
      if (rt <= 0) {
        ErrorLog << "read empty while occur read event, because of peer close, sys error=" << strerror(errno) << ", now to clear tcp connection";
        clearClient();
        // this cor can destroy
        close_flag = true;
        break;
        read_all = true;
      } else {
        if (rt == read_count) {
          // is is possible read more data, should continue read
          continue;
        } else if (rt < read_count) {
          // read all data in socket buffer, skip out loop
          read_all = true;
          break;
        } 
      }
      // m_read_buffer->writeToBuffer(buf, rt);
    }
    if (close_flag) {
      break;
    }
    if (!read_all) {
      ErrorLog << "not read all data in socket buffer";
    }
    m_reactor->addTask(std::bind(&TcpConnection::execute, this)); 
    Coroutine::Yield();
  }
}

void TcpConnection::execute() {
  while(m_read_buffer->readAble() > 0) {
    TinyPbStruct pb_struct; 
    m_codec->decode(m_read_buffer.get(), &pb_struct);
    // DebugLog << "parse service_name=" << pb_struct.service_full_name;
    if (pb_struct.decode_succ) {
      // DebugLog << "parse succ ";
      // DebugLog << "pb_data.size = " << pb_struct.pb_data.length();
      m_tcp_svr->getDispatcher()->dispatch(&pb_struct, this);
    }
  }
}

void TcpConnection::MainWriteCoFunc() {
  while(!m_stop_write) {
    if (m_state != Connected) {
      break;
    }

    if (m_write_buffer->readAble() == 0) {
      DebugLog << "app buffer of fd[" << m_fd << "] no data to write, to yiled this coroutine";
      Coroutine::Yield();
      continue;
    }
    
    int total_size = m_write_buffer->readAble();
    int read_index = m_write_buffer->readIndex();
    int rt = write(m_fd, &(m_write_buffer->getBufferVector()[read_index]), total_size);
    if (rt <= 0) {
      ErrorLog << "write empty, error=" << strerror(errno);
    }
    DebugLog << "succ write " << rt << " bytes";
    m_write_buffer->recycle(read_index + rt);
    if (m_write_buffer->readAble() == 0) {
      // if no data to write, need back main coroutine
      Coroutine::Yield();
    }
  }
}


void TcpConnection::clearClient() {
  if (m_state == Closed) {
    DebugLog << "this client has closed";
    return;
  }
  // first unregister epoll event
  m_fd_event->unregisterFromReactor(); 

  // stop read and write cor
  m_stop_read = true;
  m_stop_write = true;

  close(m_fd_event->getFd());
  m_state = Closed;

}

void TcpConnection::shutdownConnection() {
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

TcpBuffer* TcpConnection::getInBuffer() {
  return m_read_buffer.get();
}

TcpBuffer* TcpConnection::getOutBuffer() {
  return m_write_buffer.get();
}


}
