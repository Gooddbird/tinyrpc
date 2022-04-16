#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include "tinypb_rpc_controller.h"

namespace tinyrpc {

void TinyPbRpcController::Reset() {}

bool TinyPbRpcController::Failed() const {
  return false;
}

std::string TinyPbRpcController::ErrorText() const {
  return m_error_info;
}

void TinyPbRpcController::StartCancel() {}

void TinyPbRpcController::SetFailed(const std::string& reason) {
  m_error_info = reason;
}

bool TinyPbRpcController::IsCanceled() const {
  return false;
}

void TinyPbRpcController::NotifyOnCancel(google::protobuf::Closure* callback) {

}

void TinyPbRpcController::SetErrorCode(const int error_code) {
  m_error_code = error_code;
}

int TinyPbRpcController::ErrorCode() const {
  return m_error_code; 
}

std::string TinyPbRpcController::MsgSeq() const {
  return m_msg_req;
}

void TinyPbRpcController::SetMsgReq(const std::string& msg_req) {
  m_msg_req = msg_req;
}

void TinyPbRpcController::SetError(const int err_code, const std::string& err_info) {
  SetFailed(err_info);
  SetErrorCode(err_code);
}


}