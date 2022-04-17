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


//
// You should use TcpClient in a coroutine(not main coroutine)
//
class TcpClient {
 public:
  typedef std::shared_ptr<TcpClient> ptr;

  TcpClient(NetAddress::ptr addr);

  ~TcpClient();

  void init();

  int sendAndRecv();

  void stop();

  TcpConnection* getConnection();

  void setTimeout(const int v) {
    m_max_timeout = v;
    setMaxTimeOut(v); 
  }

  void setTryCounts(const int v) {
    m_try_counts = v;
  }

  const std::string& getErrInfo() {
    return m_err_info;
  }

  NetAddress::ptr getPeerAddr() const {
    return m_peer_addr;
  }

  NetAddress::ptr getLocalAddr() const {
    return m_local_addr;
  }


 private:

  int m_family;
  int m_fd {-1};
  int m_try_counts {3};         // max try reconnect times
  int m_max_timeout {5};       // max connect timeout, s
  bool m_is_stop {false};
  std::string m_err_info;      // error info of client

  NetAddress::ptr m_local_addr {nullptr};
  NetAddress::ptr m_peer_addr {nullptr};
  Reactor* m_reactor {nullptr};
  TcpConnection::ptr m_connection {nullptr};

  bool m_connect_succ {false};

}; 

}



#endif