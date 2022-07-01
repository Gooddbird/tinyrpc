#ifndef TINYRPC_NET_TINYPB_TINYPB_RPC_ASYNC_CHANNEL_H
#define TINYRPC_NET_TINYPB_TINYPB_RPC_ASYNC_CHANNEL_H 

#include <google/protobuf/service.h>
#include <future>
#include "tinyrpc/net/tinypb/tinypb_data.h"
#include "tinyrpc/net/tinypb/tinypb_rpc_channel.h"
#include "tinyrpc/net/net_address.h"
#include "tinyrpc/net/tcp/tcp_client.h"
#include "tinyrpc/coroutine/coroutine.h"

namespace tinyrpc {

class TinyPbRpcAsyncChannel : public google::protobuf::RpcChannel , public std::enable_shared_from_this<TinyPbRpcAsyncChannel> {

 public:
  typedef std::shared_ptr<TinyPbRpcAsyncChannel> ptr;

  TinyPbRpcAsyncChannel(NetAddress::ptr addr);
  ~TinyPbRpcAsyncChannel();

  void CallMethod(const google::protobuf::MethodDescriptor* method, 
      google::protobuf::RpcController* controller, 
      const google::protobuf::Message* request, 
      google::protobuf::Message* response, 
      google::protobuf::Closure* done);


  TinyPbRpcChannel* getRpcChannel();

  // std::future<bool> getFuture();

  std::future<bool> getFuture();

 private:
  TinyPbRpcChannel::ptr m_rpc_channel;
  std::promise<bool> m_promise;
  Coroutine::ptr m_cor;

 public:
  // std::future<bool> m_future;

};

}



#endif