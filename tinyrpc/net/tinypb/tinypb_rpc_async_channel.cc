#include <memory>
#include <future>
#include <google/protobuf/service.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include "tinyrpc/net/net_address.h"
#include "tinyrpc/net/tcp/io_thread.h"
#include "tinyrpc/comm/error_code.h"
#include "tinyrpc/net/tcp/tcp_client.h"
#include "tinyrpc/net/tinypb/tinypb_rpc_async_channel.h"
#include "tinyrpc/net/tinypb/tinypb_rpc_channel.h"
#include "tinyrpc/net/tinypb/tinypb_rpc_controller.h"
#include "tinyrpc/net/tinypb/tinypb_codec.h"
#include "tinyrpc/net/tinypb/tinypb_data.h"
#include "tinyrpc/comm/log.h"
#include "tinyrpc/comm/start.h"
#include "tinyrpc/coroutine/coroutine_pool.h"
#include "tinyrpc/coroutine/coroutine.h"


namespace tinyrpc {

TinyPbRpcAsyncChannel::TinyPbRpcAsyncChannel(NetAddress::ptr addr) {
  m_rpc_channel = std::make_shared<TinyPbRpcChannel>(addr);
}

TinyPbRpcAsyncChannel::~TinyPbRpcAsyncChannel() {
  DebugLog << "~TinyPbRpcAsyncChannel(), return coroutine";
  GetCoroutinePool()->returnCoroutine(m_cor);
}

TinyPbRpcChannel* TinyPbRpcAsyncChannel::getRpcChannel() {
  return m_rpc_channel.get();
}

void TinyPbRpcAsyncChannel::CallMethod(const google::protobuf::MethodDescriptor* method, 
    google::protobuf::RpcController* controller, 
    const google::protobuf::Message* request, 
    google::protobuf::Message* response, 
    google::protobuf::Closure* done) {
  
  std::shared_ptr<TinyPbRpcAsyncChannel> s_ptr = shared_from_this();
  std::shared_ptr<const google::protobuf::MethodDescriptor> method_ptr;
  method_ptr.reset(method);

  std::shared_ptr<google::protobuf::RpcController> controller_ptr;
  controller_ptr.reset(controller);

  std::shared_ptr<const google::protobuf::Message> req_ptr;
  req_ptr.reset(request);

  std::shared_ptr<google::protobuf::Message> res_ptr;
  res_ptr.reset(response);

  std::shared_ptr<google::protobuf::Closure> clo_ptr;
  clo_ptr.reset(done);

  auto cb = [s_ptr, method_ptr, controller_ptr, req_ptr, res_ptr, clo_ptr]() {
    DebugLog << "now excute rpc call method by this thread";
    s_ptr->getRpcChannel()->CallMethod(method_ptr.get(), controller_ptr.get(), req_ptr.get(), res_ptr.get(), clo_ptr.get());
    DebugLog << "excute rpc call method by this thread finish";
    s_ptr->m_promise.set_value(true);
  };

  m_cor = GetCoroutinePool()->getCoroutineInstanse();
  m_cor->setCallBack(cb);

  GetServer()->getIOThreadPool()->addCoroutineRandomThread(m_cor, false);
}

std::future<bool> TinyPbRpcAsyncChannel::getFuture() {
  return m_promise.get_future();
}


}