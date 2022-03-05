#include <google/protobuf/message.h>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>

#include "../abstract_dispatcher.h"
#include "tinypb_data.h"
#include "tinypb_rpc_dispatcher.h"

namespace tinyrpc {

void TinyPbRpcDispacther::dispatch(AbstractData* data, const TcpConnection::ptr& conn) {
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
  auto it = g_service_map.find(service_name);
  if (it == g_service_map.end()) {
    ErrorLog << "not found service_name = " << service_name;
    retrun;
  }
  std::shared_ptr<google::protobuf::Service> service = (*it).second;
  assert(service != nullptr);

  std::shared_ptr<google::protobuf::MethodDescriptor> method;
  method.reset(service->GetDescriptor()->FindMethodByName(method_name));

  std::shared_ptr<google::protobuf::Message> request;
  request.reset(service->GetRequestPrototype(method).New());

  if(!request->ParseFromArray(&(tmp->pb_data[0]), tmp->pb_data.size())) {
    ErrorLog << "parse request error";
    return;
  }

  std::shared_ptr<google::protobuf::Message> response;
  response.reset(service->GetResponsePrototype(method).New());

  std::shared_ptr<google::protobuf::RpcController> rpc_controller = shd::make_shared<google::protobuf::RpcController>();

  auto cb = [] () {
    DebugLog << "call service[" << service_name << "] succ";
  }

  service->CallMethod(method.get(), rpc_controller.get(), request.get(), response.get(), cb);

}

bool TinyPbRpcDispacther::parseServiceFullName(const std::string& full_name, std::string& service_name, std::string& method_name) {
  if (full_name.empty()) {
    ErrorLog << "service_full_name empty";
    return false;
  }
  std::size_t i = full_name.find(".");
  if (i == full_name.npos()) {
    ErrorLog << "not found [.]";
    return false;
  }

  service_name = full_name.substr(0, i);
  method_name = full_name.substr(i + 1, full_name.length() - i - 1);
  return true;

}

void TinyPbRpcDispacther::registerService(google::protobuf::Service *service) {
  std::shared_ptr<google::protobuf::Service> ptr;
  ptr.reset(service);
  std::string service_name = ptr->GetDescriptor()->full_name();
  g_service_map[service_name] = ptr;
}


};


}