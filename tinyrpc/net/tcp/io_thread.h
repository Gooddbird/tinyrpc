#ifndef TINYRPC_NET_TCP_IO_THREAD_H
#define TINYRPC_NET_TCP_IO_THREAD_H

#include <memory>
#include <map>
#include <atomic>
#include <functional>
#include "tinyrpc/net/reactor.h"
#include "tinyrpc/net/tcp/tcp_connection_time_wheel.h"
#include "tinyrpc/coroutine/coroutine.h"


namespace tinyrpc {

class TcpServer;
class TcpConection;

class IOThread {

 public:
  
  typedef std::shared_ptr<IOThread> ptr;
 	IOThread();

	~IOThread();  

  Reactor* getReactor();

  TcpTimeWheel::ptr getTimeWheel();

  bool addClient(TcpServer* tcp_svr, int fd);

  pthread_t getPthreadId();


 public:
  static IOThread* GetCurrentIOThread();

 private:
 	static void* main(void* arg);


 private:
  void MainLoopTimerFunc();

 private:
 	Reactor* m_reactor;
  std::map<int, std::shared_ptr<TcpConnection>> m_clients;

  TcpTimeWheel::ptr m_time_wheel;

	pthread_t m_thread;
	pid_t m_tid;
  TimerEvent::ptr m_timer_event;

};

class IOThreadPool {

 public:
  typedef std::shared_ptr<IOThreadPool> ptr;

  IOThreadPool(int size);

  IOThread* getIOThread();

  int getIOThreadPoolSize();

  void broadcastTask(std::function<void()> cb);

  void addTaskByIndex(int index, std::function<void()> cb);

  // add a coroutine to random thread in io thread pool
  // self = false, means random thread cann't be current thread
  void addCoroutineRandomThread(Coroutine::ptr cor, bool self = false);

 private:
  int m_size {0};

  std::atomic<int> m_index {-1};

  std::vector<IOThread::ptr> m_io_threads;
  
};


}

#endif
