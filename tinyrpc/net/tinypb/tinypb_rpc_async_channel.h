#ifndef TINYRPC_NET_TINYPB_TINYPB_RPC_ASYNC_CHANNEL_H
#define TINYRPC_NET_TINYPB_TINYPB_RPC_ASYNC_CHANNEL_H 

#include <google/protobuf/service.h>
#include <future>
#include "tinyrpc/net/tinypb/tinypb_data.h"
#include "tinyrpc/net/tinypb/tinypb_rpc_channel.h"
#include "tinyrpc/net/tinypb/tinypb_rpc_controller.h"
#include "tinyrpc/net/net_address.h"
#include "tinyrpc/net/tcp/tcp_client.h"
#include "tinyrpc/coroutine/coroutine.h"

namespace tinyrpc {

class TinyPbRpcAsyncChannel : public google::protobuf::RpcChannel , public std::enable_shared_from_this<TinyPbRpcAsyncChannel> {

 public:
  typedef std::shared_ptr<TinyPbRpcAsyncChannel> ptr;
  typedef std::shared_ptr<google::protobuf::RpcController> con_ptr;
  typedef std::shared_ptr<google::protobuf::Message> msg_ptr;
  typedef std::shared_ptr<google::protobuf::Closure> clo_ptr;

  TinyPbRpcAsyncChannel(NetAddress::ptr addr);
  ~TinyPbRpcAsyncChannel();

  void CallMethod(const google::protobuf::MethodDescriptor* method, 
      google::protobuf::RpcController* controller, 
      const google::protobuf::Message* request, 
      google::protobuf::Message* response, 
      google::protobuf::Closure* done);


  TinyPbRpcChannel* getRpcChannel();

  // must call saveCallee before CallMethod
  // in order to save shared_ptr count of req res controller
  void saveCallee(con_ptr controller, msg_ptr req, msg_ptr res, clo_ptr closure);

  void wait();

  void setFinished(bool value);

  bool getNeedResume();

  IOThread* getIOThread();

  Coroutine* getCurrentCoroutine();

  google::protobuf::RpcController* getControllerPtr();

  google::protobuf::Message* getRequestPtr();

  google::protobuf::Message* getResponsePtr();

  google::protobuf::Closure* getClosurePtr();


 private:
  TinyPbRpcChannel::ptr m_rpc_channel;
  Coroutine::ptr m_pending_cor;
  Coroutine* m_current_cor {NULL};
  IOThread* m_current_iothread {NULL};
  bool m_is_finished {false};
  bool m_need_resume {false};
  bool m_is_pre_set {false};

 private:
  con_ptr m_controller;
  msg_ptr m_req;
  msg_ptr m_res;
  clo_ptr m_closure;

};

}



#endif