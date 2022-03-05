#ifndef TINYRPC_NET_TINYPB_TINYPB_RPC_DISPATCHER_H
#define TINYRPC_NET_TINYPB_TINYPB_RPC_DISPATCHER_H

#include <google/protobuf/message.h>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <map>
#include <memory>

#include "../abstract_dispatcher.h"
#include "tinypb_data.h"


namespace tinyrpc {

class TinyPbRpcDispacther : public AbstractDispatcher {
 public:
  TinyPbRpcDispacther() {}; 
  ~TinyPbRpcDispacther() {};


  void dispatch(AbstractData* data, const TcpConnection::ptr& conn);

  bool parseServiceFullName(const std::string& full_name, std::string& service_name, std::string& method_name);

  void registerService(google::protobuf::Service* service);

 public:
  static std::map<std::string, std::shared_ptr<google::protobuf::Service>> g_service_map;

 private:

};


}



#endif