#ifndef TINYRPC_NET_TINYPB_TINYPB_RPC_CHANNEL_H
#define TINYRPC_NET_TINYPB_TINYPB_RPC_CHANNEL_H 

#include <memory>
#include <google/protobuf/service.h>
#include "tinyrpc/net/net_address.h"
#include "tinyrpc/net//tcp/tcp_client.h"

namespace tinyrpc {

class TinyPbRpcChannel : public google::protobuf::RpcChannel {

 public:
  typedef std::shared_ptr<TinyPbRpcChannel> ptr;
  TinyPbRpcChannel(NetAddress::ptr addr);
  ~TinyPbRpcChannel() = default;

void CallMethod(const google::protobuf::MethodDescriptor* method, 
    google::protobuf::RpcController* controller, 
    const google::protobuf::Message* request, 
    google::protobuf::Message* response, 
    google::protobuf::Closure* done);
 
 private:
  NetAddress::ptr m_addr;
  // TcpClient::ptr m_client;

};

}



#endif