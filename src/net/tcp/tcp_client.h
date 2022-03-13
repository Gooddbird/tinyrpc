#ifndef TINYRPC_NET_TCP_TCP_CLIENT_H
#define TINYRPC_NET_TCP_TCP_CLIENT_H

#include <memory>
#include <google/protobuf/service.h>
#include "../../coroutine/coroutine.h"
#include "../../coroutine/coroutine_hook.h"
#include "../net_address.h"
#include "../reactor.h"
#include "tcp_connection.h"

namespace tinyrpc {

class TcpClient {
 public:
  typedef std::shared_ptr<TcpClient> ptr;

  TcpClient(NetAddress::ptr addr);

  ~TcpClient();

  void start();

  void stop();

  void onReply();

  TcpConnection* getConnection();

  void setTimeout(const int v) {
    m_max_timeout = v;
    setMaxTimeOut(v); 
  }

  void setTryCounts(const int v) {
    m_try_counts = v;
  }

 private:
  void MainConnectCorFunc();


 private:

  int m_family;
  int m_fd {-1};
  int m_try_counts {3};         // max try reconnect times
  int m_max_timeout {75};       // max connect timeout, s
  NetAddress::ptr m_local_addr {nullptr};
  NetAddress::ptr m_peer_addr {nullptr};
  Reactor* m_reactor {nullptr};
  Coroutine::ptr m_connect_cor {nullptr};
  TcpConnection::ptr m_connection {nullptr};

  bool m_connect_succ {false};

}; 

}



#endif