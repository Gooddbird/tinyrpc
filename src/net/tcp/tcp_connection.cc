#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include "tcp_connection.h"
#include "tcp_server.h"
#include "tcp_client.h"
#include "../tinypb/tinypb_codec.h"
#include "../tinypb/tinypb_data.h"
#include "../../coroutine/coroutine_hook.h"
#include "../../coroutine/coroutine_pool.h"
#include "tcp_connection_time_wheel.h"
#include "abstract_slot.h"
#include "../timer.h"

namespace tinyrpc {

TcpConnection::TcpConnection(tinyrpc::TcpServer* tcp_svr, tinyrpc::IOThread* io_thread, int fd, int buff_size, NetAddress::ptr peer_addr)
  : m_io_thread(io_thread), m_fd(fd), m_state(Connected), m_connection_type(ServerConnection), m_peer_addr(peer_addr) {	
  m_reactor = m_io_thread->getReactor();

  assert(tcp_svr!= nullptr); 
  m_tcp_svr = tcp_svr;

  // tinyrpc::enableHook();
  m_fd_event = FdEventContainer::GetFdContainer()->getFdEvent(fd);
  m_fd_event->setReactor(m_reactor);
  initBuffer(buff_size); 

  m_read_cor = GetCoroutinePool()->getCoroutineInstanse();
  m_read_cor->setCallBack(std::bind(&TcpConnection::MainReadCoFunc, this));

  m_write_cor = GetCoroutinePool()->getCoroutineInstanse();
  m_write_cor->setCallBack(std::bind(&TcpConnection::MainWriteCoFunc, this));

  m_reactor->addCoroutine(m_read_cor);
  
  m_codec = std::make_shared<TinyPbCodeC>();
  DebugLog << "succ create tcp connection";
}

TcpConnection::TcpConnection(tinyrpc::TcpClient* tcp_cli, tinyrpc::Reactor* reactor, int fd, int buff_size, NetAddress::ptr peer_addr)
  : m_fd(fd), m_state(NotConnected), m_connection_type(ClientConnection), m_peer_addr(peer_addr) {
  assert(reactor != nullptr); 
  m_reactor = reactor;

  assert(tcp_cli!= nullptr); 
  m_tcp_cli = tcp_cli;

  // tinyrpc::enableHook();
  m_fd_event = FdEventContainer::GetFdContainer()->getFdEvent(fd);
  m_fd_event->setReactor(m_reactor);
  initBuffer(buff_size); 

  m_read_cor = std::make_shared<Coroutine>(128 * 1024, std::bind(&TcpConnection::MainReadCoFunc, this));
  m_write_cor = std::make_shared<Coroutine>(128 * 1024, std::bind(&TcpConnection::MainWriteCoFunc, this));
  // m_reactor->addCoroutine(m_read_cor);
  
  m_codec = std::make_shared<TinyPbCodeC>();

  DebugLog << "succ create tcp connection[NotConnected]";

}

void TcpConnection::registerToTimeWheel() {
  auto cb = [] (TcpConnection::ptr conn) {
    conn->shutdownConnection();
  };
  // m_conn_slot = new AbstractSlot<TcpConnection>(shared_from_this(), cb);
  TcpTimeWheel::TcpConnectionSlot::ptr tmp = std::make_shared<AbstractSlot<TcpConnection>>(shared_from_this(), cb);
  m_weak_slot = tmp;
  m_io_thread->getTimeWheel()->fresh(tmp);

}

void TcpConnection::setUpClient() {
  m_reactor->addCoroutine(m_read_cor);
  m_state = Connected;
}

TcpConnection::~TcpConnection() {
  GetCoroutinePool()->returnCoroutine(m_read_cor->getCorId());
  GetCoroutinePool()->returnCoroutine(m_write_cor->getCorId());

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
    DebugLog << Coroutine::GetCurrentCoroutine();
    return;
  }
  //
  Coroutine::Resume(m_write_cor.get());
}

void TcpConnection::MainReadCoFunc() {
  while(!m_stop_read) {
    if (m_state == Closed || m_state == NotConnected) {
      break;
    }
    bool read_all = false;
    bool close_flag = false;
    int count = 0;
    while (!read_all) {

      if (m_read_buffer->writeAble() == 0) {
        m_read_buffer->resizeBuffer(2 * m_read_buffer->getSize());
      }

      int read_count = m_read_buffer->writeAble();
      int write_index = m_read_buffer->writeIndex();

      DebugLog << "m_read_buffer size=" << m_read_buffer->getBufferVector().size() << "rd=" << m_read_buffer->readIndex() << "wd=" << m_read_buffer->writeIndex();
      int rt = read_hook(m_fd, &(m_read_buffer->m_buffer[write_index]), read_count);
      m_read_buffer->recycleWrite(rt);
      DebugLog << "m_read_buffer size=" << m_read_buffer->getBufferVector().size() << "rd=" << m_read_buffer->readIndex() << "wd=" << m_read_buffer->writeIndex();

      DebugLog << "read data back";
      count += rt;
      if (rt <= 0) {
        DebugLog << "rt <= 0";
        ErrorLog << "read empty while occur read event, because of peer close, sys error=" << strerror(errno) << ", now to clear tcp connection";
        clearClient();
        // this cor can destroy
        close_flag = true;
        break;
        read_all = true;
      } else {
        if (rt == read_count) {
          DebugLog << "read_count == rt";
          // is is possible read more data, should continue read
          continue;
        } else if (rt < read_count) {
          DebugLog << "read_count > rt";
          // read all data in socket buffer, skip out loop
          read_all = true;
          break;
        } 
      }
    }
    if (close_flag) {
      break;
    }
    if (!read_all) {
      ErrorLog << "not read all data in socket buffer";
    }
    InfoLog << "recv [" << count << "] bytes data from [" << m_peer_addr->toString() << "], fd [" << m_fd << "]";
    if (m_connection_type == ServerConnection) {
      TcpTimeWheel::TcpConnectionSlot::ptr tmp = m_weak_slot.lock();
      if (tmp) {
        m_io_thread->getTimeWheel()->fresh(tmp);
      }
    }
    m_reactor->addTask(std::bind(&TcpConnection::execute, this)); 
    Coroutine::Yield();
    DebugLog << "read yield back";
  }
}

void TcpConnection::execute() {
  DebugLog << "begin to do execute";

  // it only server do this
  while(m_read_buffer->readAble() > 0) {
    TinyPbStruct pb_struct; 
    m_codec->decode(m_read_buffer.get(), &pb_struct);
    // DebugLog << "parse service_name=" << pb_struct.service_full_name;
    if (!pb_struct.decode_succ) {
      break;
    }
    if (m_connection_type == ServerConnection) {
      DebugLog << "to dispatch this package";
      m_tcp_svr->getDispatcher()->dispatch(&pb_struct, this);
      DebugLog << "contine parse next package";
    } else if (m_connection_type == ClientConnection) {
      // TODO:
      m_client_res_data_queue.push(&pb_struct);
      // m_tcp_cli->stop();
      // m_tcp_
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
    int rt = write_hook(m_fd, &(m_write_buffer->m_buffer[read_index]), total_size);
    // InfoLog << "write end";
    if (rt <= 0) {
      ErrorLog << "write empty, error=" << strerror(errno);
    }
    DebugLog << "succ write " << rt << " bytes";
    m_write_buffer->recycleRead(rt);
    DebugLog << "recycle write index =" << m_write_buffer->writeIndex() << ", read_index =" << m_write_buffer->readIndex() << "readable = " << m_write_buffer->readAble();
    InfoLog << "send[" << rt << "] bytes data to [" << m_peer_addr->toString() << "], fd [" << m_fd << "]";
    if (m_write_buffer->readAble() <= 0) {
      DebugLog << "send all data, now unregister write event on reactor and yield Coroutine";
      m_fd_event->delListenEvents(IOEvent::WRITE);
      // if no data to write, need back main coroutine
      Coroutine::Yield();
      DebugLog << "write cor back";
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
  if (m_state == Closed || m_state == NotConnected) {
    DebugLog << "this client has closed";
    return;
  }
  m_state = HalfClosing; 
  InfoLog << "shutdown conn[" << m_peer_addr->toString() << "]";
  // call sys shutdown to send FIN
  // wait client done something, client will send FIN
  // and fd occur read event but byte count is 0
  // then will call clearClient to set CLOSED
  // IOThread::MainLoopTimerFunc will delete CLOSED connection
  shutdown(m_fd_event->getFd(), SHUT_RDWR);

}

TcpBuffer* TcpConnection::getInBuffer() {
  return m_read_buffer.get();
}

TcpBuffer* TcpConnection::getOutBuffer() {
  return m_write_buffer.get();
}

const TinyPbStruct* TcpConnection::getResPackageData() {
  return &m_client_res_data;
}


TinyPbCodeC* TcpConnection::getCodec() const {
  return m_codec.get();
}

TcpConnectionState TcpConnection::getState() const {
  return m_state;
}

}
