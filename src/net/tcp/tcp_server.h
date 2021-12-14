#ifndef TINYRPC_NET_TCP_TCP_SERVER_H
#define TINYRPC_NET_TCP_TCP_SERVER_H

#include "io_thread.h"
#include "../reactor.h"
#include "../fd_event.h"
#include "../timer.h"
#include "../net_address.h"


namespace tinyrpc {

class TcpAcceptor : public FdEvent {

 public:

  typedef std::shared_ptr<TcpAcceptor> ptr;
  TcpAcceptor(Reactor* reactor, NetAddress::ptr net_addr);

  void init();

  ~TcpAcceptor();

 
 private:
  int m_family;
  NetAddress::ptr m_net_addr;

};



class TcpServer {

 public:

	TcpServer(NetAddress::ptr addr);

  ~TcpServer();

  void init();


 private:
  void onReadCallBack();

  void onWriteCallBack();



 private:
  
  NetAddress::ptr m_addr;

  TcpAcceptor::ptr m_acceptor;
  pid_t m_main_thread_id {0};
  Timer* m_timer;
  std::vector<IOThread::ptr> m_io_threads;

  Reactor::ptr m_main_reactor;

};

}

#endif
