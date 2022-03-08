#include <google/protobuf/message.h>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>

#include "../abstract_dispatcher.h"
#include "tinypb_data.h"
#include "tinypb_rpc_dispatcher.h"
#include "tinypb_rpc_controller.h"
#include "tinypb_rpc_closure.h"

namespace tinyrpc {

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
  DebugLog << "request.name = " << request->GetDescriptor()->full_name();
  // DebugLog << "req pb_data_size = " << tmp->pb_data.length();

  if(!request->ParseFromString(tmp->pb_data)) {
    ErrorLog << "parse request error";
    return;
  }

  google::protobuf::Message* response = service->GetResponsePrototype(method).New();

  DebugLog << "response.name = " << response->GetDescriptor()->full_name();

  TinyPbRpcController* rpc_controller = new TinyPbRpcController();

  std::function<void()> cb = [tmp]()
  {
    DebugLog << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
    DebugLog << "call [" << tmp->service_full_name << "] succ";
    DebugLog << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>";
  };

  TinyPbRpcClosure closure(cb);
  service->CallMethod(method, rpc_controller, request, response, &closure);

  // delete method;
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
  DebugLog << "service_name = " << service_name;
  method_name = full_name.substr(i + 1, full_name.length() - i - 1);
  DebugLog << "method_name = " << method_name;

  return true;

}

void TinyPbRpcDispacther::registerService(google::protobuf::Service *service) {
  std::string service_name = service->GetDescriptor()->full_name();
  m_service_map[service_name] = service;
  InfoLog << "succ register service[" << service_name << "]!"; 
}

}