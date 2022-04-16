#ifndef TINYRPC_NET_TINYPB_TINYPB_RPC_CONRTOLLER_H
#define TINYRPC_NET_TINYPB_TINYPB_RPC_CONRTOLLER_H

#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include "../net_address.h"

namespace tinyrpc {

class TinyPbRpcController : public google::protobuf::RpcController {

 public:

  // Client-side methods ---------------------------------------------

  TinyPbRpcController() = default;

  ~TinyPbRpcController() = default;

  void Reset() override;

  bool Failed() const override;


  // Server-side methods ---------------------------------------------

  std::string ErrorText() const override;

  void StartCancel() override;

  void SetFailed(const std::string& reason) override;

  bool IsCanceled() const override;

  void NotifyOnCancel(google::protobuf::Closure* callback) override;

  int ErrorCode() const;

  void SetErrorCode(const int error_code);

  std::string MsgSeq() const;

  void SetMsgReq(const std::string& msg_req);

  void SetError(const int err_code, const std::string& err_info);
  

 private:
  int m_error_code {0};
  std::string m_error_info;
  std::string m_msg_req;
  bool m_is_failed {false};
  bool m_is_cancled {false};
  NetAddress::ptr m_peer_addr;
  NetAddress::ptr m_local_addr;


};

}


#endif