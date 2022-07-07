#ifndef TINYRPC_NET_TCP_TCP_SERVER_H
#define TINYRPC_NET_TCP_TCP_SERVER_H

#include <map>
#include <google/protobuf/service.h>
#include "tinyrpc/net/reactor.h"
#include "tinyrpc/net/fd_event.h"
#include "tinyrpc/net/timer.h"
#include "tinyrpc/net/net_address.h"
#include "tinyrpc/net/tcp/tcp_connection.h"
#include "tinyrpc/net/tcp/io_thread.h"
#include "tinyrpc/net/tcp/tcp_connection_time_wheel.h"
#include "tinyrpc/net/abstract_codec.h"
#include "tinyrpc/net/abstract_dispatcher.h"
#include "tinyrpc/net/http/http_dispatcher.h"
#include "tinyrpc/net/http/http_servlet.h"


namespace tinyrpc {

class TcpAcceptor {

 public:

  typedef std::shared_ptr<TcpAcceptor> ptr;
  TcpAcceptor(NetAddress::ptr net_addr);

  void init();

  int toAccept();

  ~TcpAcceptor();

  NetAddress::ptr getPeerAddr() {
    return m_peer_addr;
  }

  NetAddress::ptr geLocalAddr() {
    return m_local_addr;
  }
 
 private:
  int m_family;
  int m_fd;

  NetAddress::ptr m_local_addr;
  NetAddress::ptr m_peer_addr;

};


class TcpServer {

 public:

  typedef std::shared_ptr<TcpServer> ptr;

	TcpServer(NetAddress::ptr addr, ProtocalType type = TinyPb_Protocal);

  ~TcpServer();

  void start();

  void addCoroutine(tinyrpc::Coroutine::ptr cor);

  bool addClient(int fd);

  AbstractDispatcher::ptr getDispatcher();

  AbstractCodeC::ptr getCodec();

  TcpTimeWheel* getTimeWheel();

  NetAddress::ptr getPeerAddr();

  bool registerService(std::shared_ptr<google::protobuf::Service> service);

  bool registerHttpServlet(const std::string& url_path, HttpServlet::ptr servlet);

  IOThreadPool::ptr getIOThreadPool();

 private:
  void MainAcceptCorFunc();

 private:
  
  NetAddress::ptr m_addr;

  TcpAcceptor::ptr m_acceptor;

  int m_tcp_counts {0};

  Reactor* m_main_reactor {nullptr};

  bool m_is_stop_accept {false};

  Coroutine::ptr m_accept_cor;
  
  AbstractDispatcher::ptr m_dispatcher;

  AbstractCodeC::ptr m_codec;

  IOThreadPool::ptr m_io_pool;

  ProtocalType m_protocal_type {TinyPb_Protocal};


};

}

#endif
