#include <google/protobuf/message.h>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>

#include "../abstract_dispatcher.h"
#include "tinypb_data.h"
#include "tinypb_rpc_dispatcher.h"
#include "tinypb_rpc_controller.h"

namespace tinyrpc {

void cb() {
  DebugLog << "call succ";
}

TinyPbRpcDispacther::TinyPbRpcDispacther() {

}

TinyPbRpcDispacther::~TinyPbRpcDispacther() {

}

void TinyPbRpcDispacther::dispatch(AbstractData* data, TcpConnection* conn) {
  TinyPbStruct* tmp = dynamic_cast<TinyPbStruct*>(data);

  if (tmp == nullptr) {
    ErrorLog << "dynamic_cast error";
  }
  assert(tmp != nullptr);
  std::string service_name;
  std::string method_name;
  if (!parseServiceFullName(tmp->service_full_name, service_name, method_name)) {
    ErrorLog << "parse service name error, return";
    return;
  }
  auto it = m_service_map.find(service_name);
  if (it == m_service_map.end()) {
    ErrorLog << "not found service_name = " << service_name;
    return;
  }
  google::protobuf::Service* service = (*it).second;
  assert(service != nullptr);

  const google::protobuf::MethodDescriptor* method = service->GetDescriptor()->FindMethodByName(method_name);

  google::protobuf::Message* request = service->GetRequestPrototype(method).New();

  if(!request->ParseFromArray(&(tmp->pb_data[0]), tmp->pb_data.size())) {
    ErrorLog << "parse request error";
    return;
  }

  google::protobuf::Message* response = service->GetResponsePrototype(method).New();

  TinyPbRpcController* rpc_controller = new TinyPbRpcController();

  google::protobuf::Closure* call_back = google::protobuf::NewCallback(&cb);
  service->CallMethod(method, rpc_controller, request, response, call_back);

  delete method;
  delete request;
  delete response;
}



bool TinyPbRpcDispacther::parseServiceFullName(const std::string& full_name, std::string& service_name, std::string& method_name) {
  if (full_name.empty()) {
    ErrorLog << "service_full_name empty";
    return false;
  }
  std::size_t i = full_name.find(".");
  if (i == full_name.npos) {
    ErrorLog << "not found [.]";
    return false;
  }

  service_name = full_name.substr(0, i);
  method_name = full_name.substr(i + 1, full_name.length() - i - 1);
  return true;

}

void TinyPbRpcDispacther::registerService(google::protobuf::Service *service) {
  std::string service_name = service->GetDescriptor()->full_name();
  m_service_map[service_name] = service;
}

}