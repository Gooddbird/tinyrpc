#ifndef TINYRPC_NET_HTTP_HTTP_DISPATCHER_H
#define TINYRPC_NET_HTTP_HTTP_DISPATCHER_H

#include <map>
#include <memory>
#include <google/protobuf/service.h>
#include "tinyrpc/net/abstract_dispatcher.h"


namespace tinyrpc {

class HttpDispacther : public AbstractDispatcher {
 public:
  HttpDispacther() = default;

  ~HttpDispacther() = default;

  void dispatch(AbstractData* data, TcpConnection* conn);

  void registerService(google::protobuf::Service* service);

 public:

};


}



#endif