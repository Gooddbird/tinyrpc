#ifndef TINYRPC_NET_TINYPB_TINYPB_RPC_CONRTOLLER_H
#define TINYRPC_NET_TINYPB_TINYPB_RPC_CONRTOLLER_H

#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>

namespace tinyrpc {

class TinyPbRpcController : public google::protobuf::RpcController {

 public:

  TinyPbRpcController() {}

  ~TinyPbRpcController() {}

  void Reset() {}

  bool Failed() const {
    return false;
  }

  std::string ErrorText() const {
    std::string s = "";
    return s;
  }

  void StartCancel() {}

  void SetFailed(const std::string& reason) {}

  bool IsCanceled() const {
    return false;
  }

  void NotifyOnCancel(google::protobuf::Closure* callback) {

  }


};

}


#endif