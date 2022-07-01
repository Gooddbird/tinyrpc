#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include "tinypb_rpc_controller.h"

namespace tinyrpc {

void TinyPbRpcController::Reset() {}

bool TinyPbRpcController::Failed() const {
  return m_is_failed;
}

std::string TinyPbRpcController::ErrorText() const {
  return m_error_info;
}

void TinyPbRpcController::StartCancel() {}

void TinyPbRpcController::SetFailed(const std::string& reason) {
  m_is_failed = true;
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

const std::string& TinyPbRpcController::MsgSeq() const {
  return m_msg_req;
}

void TinyPbRpcController::SetMsgReq(const std::string& msg_req) {
  m_msg_req = msg_req;
}

void TinyPbRpcController::SetError(const int err_code, const std::string& err_info) {
  SetFailed(err_info);
  SetErrorCode(err_code);
}

void TinyPbRpcController::SetPeerAddr(NetAddress::ptr addr) {
  m_peer_addr = addr;
}

void TinyPbRpcController::SetLocalAddr(NetAddress::ptr addr) {
  m_local_addr = addr;
}
NetAddress::ptr TinyPbRpcController::PeerAddr() {
  return m_peer_addr;
}
  
NetAddress::ptr TinyPbRpcController::LocalAddr() {
  return m_local_addr;
}

void TinyPbRpcController::SetTimeout(const int timeout) {
  m_timeout = timeout;
}
int TinyPbRpcController::Timeout() const {
  return m_timeout;
}

void TinyPbRpcController::SetMethodName(const std::string& name) {
  m_method_name = name;
}

std::string TinyPbRpcController::GetMethodName() {
  return m_method_name;
}

void TinyPbRpcController::SetMethodFullName(const std::string& name) {
  m_full_name = name;
}

std::string TinyPbRpcController::GetMethodFullName() {
  return m_full_name;
}


}