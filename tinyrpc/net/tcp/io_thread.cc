#include <memory>
#include <map>
#include "../reactor.h"
#include "io_thread.h"
#include "tcp_connection.h"
#include "tcp_server.h"
#include "tcp_connection_time_wheel.h"
#include "../../coroutine/coroutine.h"
#include "../../comm/config.h"


namespace tinyrpc {

extern tinyrpc::Config::ptr gRpcConfig;

static thread_local Reactor* t_reactor_ptr = nullptr;

IOThread::IOThread() {
  pthread_create(&m_thread, nullptr, &IOThread::main, this);
}

IOThread::~IOThread() {
  m_reactor->stop();
  pthread_join(m_thread, nullptr);

  if (m_reactor != nullptr) {

    delete m_reactor;
    m_reactor = nullptr;
  }
}

Reactor* IOThread::getReactor() {
  return m_reactor;
}

TcpTimeWheel::ptr IOThread::getTimeWheel() {
  return m_time_wheel;
}

void* IOThread::main(void* arg) {
  // assert(t_reactor_ptr == nullptr);

  t_reactor_ptr = new Reactor(); 
  IOThread* thread = static_cast<IOThread*>(arg);
  thread->m_reactor = t_reactor_ptr;

  thread->m_timer_event = std::make_shared<TimerEvent>(10000, true, 
    std::bind(&IOThread::MainLoopTimerFunc, thread));
  
  thread->getReactor()->getTimer()->addTimerEvent(thread->m_timer_event);
  thread->m_time_wheel = std::make_shared<TcpTimeWheel>(thread->m_reactor, gRpcConfig->m_timewheel_bucket_num, gRpcConfig->m_timewheel_inteval);

  Coroutine::GetCurrentCoroutine();

  t_reactor_ptr->loop();

  return nullptr;
}

bool IOThread::addClient(TcpServer* tcp_svr, int fd) {

  auto it = m_clients.find(fd);
  if (it != m_clients.end()) {
    TcpConnection::ptr s_conn = it->second;
    if (s_conn && s_conn.use_count() > 0 && s_conn->getState() != Closed) {
      ErrorLog << "insert error, this fd of TcpConection exist and state not Closed";
      return false;
    }
    // src Tcpconnection can delete
    s_conn.reset();
		it->second.reset();
    // set new Tcpconnection	
		it->second = std::make_shared<TcpConnection>(tcp_svr, this, fd, 128, tcp_svr->getPeerAddr());
    it->second->registerToTimeWheel();

  } else {
    TcpConnection::ptr conn = std::make_shared<TcpConnection>(tcp_svr, this, fd, 128, tcp_svr->getPeerAddr()); 
    m_clients.insert(std::make_pair(fd, conn));
    conn->registerToTimeWheel();
    
  }
  return true;
}

void IOThread::MainLoopTimerFunc() {
  // DebugLog << "this IOThread loop timer excute";
  
  // delete Closed TcpConnection per loop
  // for free memory
	// DebugLog << "m_clients.size=" << m_clients.size();
  for (auto &i : m_clients) {
    // TcpConnection::ptr s_conn = i.second;
		// DebugLog << "state = " << s_conn->getState();
    if (i.second && i.second.use_count() > 0 && i.second->getState() == Closed) {
      // need to delete TcpConnection
      DebugLog << "TcpConection [fd:" << i.first << "] will delete";
      (i.second).reset();
      // s_conn.reset();
    }
	
  }
}


IOThreadPool::IOThreadPool(int size) : m_size(size) {
  m_io_threads.resize(size);
  for (int i = 0; i < size; ++i) {
    m_io_threads[i] = std::make_shared<IOThread>();
  }
}

IOThread* IOThreadPool::getIOThread() {
  if (m_index == m_size || m_index == -1) {
    m_index = 0;
  }
  return m_io_threads[m_index++].get();
}


}