#ifndef TINYRPC_NET_TCP_TCP_SERVER_H
#define TINYRPC_NET_TCP_TCP_SERVER_H

#include "io_thread.h"
#include "../reactor.h"
#include "../fd_event.h"
#include "../net_address.h"


namespace tinyrpc {

class TcpAcceptor : public FdEvent {

 public:
  TcpAcceptor(Reactor* reactor, NetAddress::ptr net_addr);

  ~TcpAcceptor();

 
 private:
  int m_family;
  NetAddress::ptr m_net_addr;

};



class TcpServer {

 public:

	explict TcpServer(Reactor::ptr m_reactor, NetAddress::ptr addr);

  void onAccept();



 private:
  
  std::vector<IOThread::ptr> m_io_threads;
  Reactor::ptr m_main_reactor;

};

}

#endif
