#ifndef TINYRPC_NET_ABSTRACT_DISPATCHER_H
#define TINYRPC_NET_ABSTRACT_DISPATCHER_H

#include "abstract_data.h"
#include "tcp/tcp_connection.h"

namespace tinyrpc {

class TcpConnection;

class AbstractDispatcher {
 public:
  AbstractDispatcher() {}
  virtual ~AbstractDispatcher() {}
  virtual void dispatch(AbstractData* data, TcpConnection* conn) = 0;
};

}


#endif
