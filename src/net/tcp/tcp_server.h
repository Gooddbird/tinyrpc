#ifndef TINYRPC_NET_TCP_TCP_SERVER_H
#define TINYRPC_NET_TCP_TCP_SERVER_H

#include <map>
#include "io_thread.h"
#include "../reactor.h"
#include "../fd_event.h"
#include "../timer.h"
#include "../net_address.h"
#include "tcp_connection.h"
#include "io_thread.h"
#include "../tinypb/tinypb_rpc_dispatcher.h"


namespace tinyrpc {

class TcpAcceptor {

 public:

  typedef std::shared_ptr<TcpAcceptor> ptr;
  TcpAcceptor(NetAddress::ptr net_addr);

  void init();

  int toAccept();

  ~TcpAcceptor();
 
 private:
  int m_family;
  int m_fd;

  NetAddress::ptr m_local_addr;
  NetAddress::ptr m_peer_addr;

};


class TcpServer {

 public:

	TcpServer(NetAddress::ptr addr, int pool_size = 2);

  ~TcpServer();

  void start();

  bool addClient(int fd, const TcpConnection::ptr& conn);

  TinyPbRpcDispacther* getDispatcher();

 private:
  void MainAcceptCorFunc();

  void MainLoopTimerFunc();

 private:
  
  NetAddress::ptr m_addr;

  TcpAcceptor::ptr m_acceptor;

  int m_tcp_counts {0};

  Reactor* m_main_reactor;

  std::map<int, TcpConnection::ptr> m_clients;

  bool m_is_stop_accept {false};

  Coroutine::ptr m_accept_cor;
  
  TimerEvent::ptr m_timer_event;
  Timer::ptr m_timer;
  TinyPbRpcDispacther::ptr m_dispatcher;

  IOThreadPool::ptr m_io_pool;

};

}

#endif
