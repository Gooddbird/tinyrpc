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
#include "tinyrpc/comm/run_time.h"
#include "tinyrpc/comm/msg_req.h"
#include "tinyrpc/coroutine/coroutine_pool.h"
#include "tinyrpc/coroutine/coroutine.h"


namespace tinyrpc {

TinyPbRpcAsyncChannel::TinyPbRpcAsyncChannel(NetAddress::ptr addr) {
  m_rpc_channel = std::make_shared<TinyPbRpcChannel>(addr);
  m_current_iothread = IOThread::GetCurrentIOThread();
  m_current_cor = Coroutine::GetCurrentCoroutine();
}

TinyPbRpcAsyncChannel::~TinyPbRpcAsyncChannel() {
  // DebugLog << "~TinyPbRpcAsyncChannel(), return coroutine";
  GetCoroutinePool()->returnCoroutine(m_pending_cor);
}

TinyPbRpcChannel* TinyPbRpcAsyncChannel::getRpcChannel() {
  return m_rpc_channel.get();
}

void TinyPbRpcAsyncChannel::saveCallee(con_ptr controller, msg_ptr req, msg_ptr res, clo_ptr closure) {
  m_controller = controller;
  m_req = req;
  m_res = res;
  m_closure = closure;
  m_is_pre_set = true;
}

void TinyPbRpcAsyncChannel::CallMethod(const google::protobuf::MethodDescriptor* method, 
    google::protobuf::RpcController* controller, 
    const google::protobuf::Message* request, 
    google::protobuf::Message* response, 
    google::protobuf::Closure* done) {
  
  TinyPbRpcController* rpc_controller = dynamic_cast<TinyPbRpcController*>(controller);
  if (!m_is_pre_set) {
    ErrorLog << "Error! must call [saveCallee()] function before [CallMethod()]"; 
    TinyPbRpcController* rpc_controller = dynamic_cast<TinyPbRpcController*>(controller);
    rpc_controller->SetError(ERROR_NOT_SET_ASYNC_PRE_CALL, "Error! must call [saveCallee()] function before [CallMethod()];");
    m_is_finished = true;
    return;
  }
  RunTime* run_time = getCurrentRunTime();
  if (run_time) {
    rpc_controller->SetMsgReq(run_time->m_msg_no);
    DebugLog << "get from RunTime succ, msgno=" << run_time->m_msg_no;
  } else {
    rpc_controller->SetMsgReq(MsgReqUtil::genMsgNumber());
    DebugLog << "get from RunTime error, generate new msgno=" << rpc_controller->MsgSeq();
  }

  std::shared_ptr<TinyPbRpcAsyncChannel> s_ptr = shared_from_this();

  auto cb = [s_ptr, method]() mutable {
    DebugLog << "now excute rpc call method by this thread";
    s_ptr->getRpcChannel()->CallMethod(method, s_ptr->getControllerPtr(), s_ptr->getRequestPtr(), s_ptr->getResponsePtr(), NULL);

    DebugLog << "excute rpc call method by this thread finish";

    auto call_back = [s_ptr]() mutable {
      DebugLog << "async excute rpc call method back old thread";
      // callback function excute in origin thread
      if (s_ptr->getClosurePtr() != nullptr) {
        s_ptr->getClosurePtr()->Run();
      }
      s_ptr->setFinished(true);

      if (s_ptr->getNeedResume()) {
        DebugLog << "async excute rpc call method back old thread, need resume";
        Coroutine::Resume(s_ptr->getCurrentCoroutine());
      }
      s_ptr.reset();
    };

    s_ptr->getIOThread()->getReactor()->addTask(call_back, true);
    s_ptr.reset();
  };
  m_pending_cor = GetServer()->getIOThreadPool()->addCoroutineToRandomThread(cb, false);

}

void TinyPbRpcAsyncChannel::wait() {
  m_need_resume = true;
  if (m_is_finished) {
    return;
  }
  Coroutine::Yield();
}

void TinyPbRpcAsyncChannel::setFinished(bool value) {
  m_is_finished = true;
}

IOThread* TinyPbRpcAsyncChannel::getIOThread() {
  return m_current_iothread;
}

Coroutine* TinyPbRpcAsyncChannel::getCurrentCoroutine() {
  return m_current_cor;
}

bool TinyPbRpcAsyncChannel::getNeedResume() {
  return m_need_resume;
}

google::protobuf::RpcController* TinyPbRpcAsyncChannel::getControllerPtr() {
  return m_controller.get();
}

google::protobuf::Message* TinyPbRpcAsyncChannel::getRequestPtr() {
  return m_req.get();
}

google::protobuf::Message* TinyPbRpcAsyncChannel::getResponsePtr() {
  return m_res.get();
}

google::protobuf::Closure* TinyPbRpcAsyncChannel::getClosurePtr() {
  return m_closure.get();
}

}