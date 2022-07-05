#include <memory>
#include <map>
#include <time.h>
#include <stdlib.h>
#include "tinyrpc/net/reactor.h"
#include "tinyrpc/net/tcp/io_thread.h"
#include "tinyrpc/net/tcp/tcp_connection.h"
#include "tinyrpc/net/tcp/tcp_server.h"
#include "tinyrpc/net/tcp/tcp_connection_time_wheel.h"
#include "tinyrpc/coroutine/coroutine.h"
#include "tinyrpc/coroutine/coroutine_pool.h"
#include "tinyrpc/comm/config.h"


namespace tinyrpc {

extern tinyrpc::Config::ptr gRpcConfig;

static thread_local Reactor* t_reactor_ptr = nullptr;

static thread_local IOThread* t_cur_io_thread = nullptr;


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

IOThread* IOThread::GetCurrentIOThread() {
  return t_cur_io_thread;
}

Reactor* IOThread::getReactor() {
  return m_reactor;
}

pthread_t IOThread::getPthreadId() {
  return m_thread;
}

TcpTimeWheel::ptr IOThread::getTimeWheel() {
  return m_time_wheel;
}

void IOThread::setThreadIndex(const int index) {
  m_index = index;
}

int IOThread::getThreadIndex() {
  return m_index;
}

void* IOThread::main(void* arg) {
  // assert(t_reactor_ptr == nullptr);

  t_reactor_ptr = new Reactor();
  IOThread* thread = static_cast<IOThread*>(arg);
  t_cur_io_thread = thread;
  thread->m_reactor = t_reactor_ptr;
  thread->m_tid = gettid();

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
    m_io_threads[i]->setThreadIndex(i);
  }
}

IOThread* IOThreadPool::getIOThread() {
  if (m_index == m_size || m_index == -1) {
    m_index = 0;
  }
  return m_io_threads[m_index++].get();
}


int IOThreadPool::getIOThreadPoolSize() {
  return m_size;
}

void IOThreadPool::broadcastTask(std::function<void()> cb) {
  for (auto i : m_io_threads) {
    i->getReactor()->addTask(cb, true);
  }
}

void IOThreadPool::addTaskByIndex(int index, std::function<void()> cb) {
  if (index >= 0 && index < m_size) {
    m_io_threads[index]->getReactor()->addTask(cb, true);
  }
}

void IOThreadPool::addCoroutineToRandomThread(Coroutine::ptr cor, bool self /* = false*/) {
  srand(time(0));
  int i = 0;
  while (1) {
    i = rand() % (m_size);
    if (!self && m_io_threads[i]->getPthreadId() == t_cur_io_thread->getPthreadId()) {
      i++;
      if (i == m_size) {
        i -= 2;
      }
    }
    break;
  }
  m_io_threads[i]->getReactor()->addCoroutine(cor, true);
  // if (m_io_threads[m_index]->getPthreadId() == t_cur_io_thread->getPthreadId()) {
  //   m_index++;
  //   if (m_index == m_size || m_index == -1) {
  //     m_index = 0;
  //   }
  // }
}


Coroutine::ptr IOThreadPool::addCoroutineToRandomThread(std::function<void()> cb, bool self/* = false*/) {
  Coroutine::ptr cor = GetCoroutinePool()->getCoroutineInstanse();
  cor->setCallBack(cb);
  addCoroutineToRandomThread(cor, self);
  return cor;
}


}