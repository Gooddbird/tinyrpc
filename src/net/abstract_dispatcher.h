#ifndef TINYRPC_NET_ABSTRACT_DISPATCHER_H
#define TINYRPC_NET_ABSTRACT_DISPATCHER_H

#include "abstract_data.h"
#include "tcp/tcp_connection.h"

namespace tinyrpc {

class AbstractDispatcher {
 public:
  AbstractDispatcher() {}
  virtual ~AbstractDispatcher() = 0;
  virtual void dispatch(AbstractData* data, const TcpConnection::ptr& conn);

};

}


#endif
